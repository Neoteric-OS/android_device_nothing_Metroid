/*
 * Copyright (C) 2026 Neoteric OS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "sensors.nothing"

#include "ProximitySensorExtension.h"

#include <fcntl.h>
#include <hardware_legacy/power.h>
#include <log/log.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <unordered_set>

struct _noth_sensor_extension_ {
    void* client = nullptr;
    float floatResponse[15] = {};
    int intResponse[15] = {};
};

static_assert(offsetof(_noth_sensor_extension_, intResponse) == 0x44);
static_assert(sizeof(_noth_sensor_extension_) == 0x80);

int noth_sensor_extension_create_client(_noth_sensor_extension_*, int);
int noth_sensor_extension_exec_sync_int_int(_noth_sensor_extension_*, int, int, int*, int*);
int noth_sensor_extension_release_client(_noth_sensor_extension_*);

namespace android {
namespace hardware {
namespace sensors {
namespace V2_1 {
namespace implementation {
namespace {

using namespace std::chrono_literals;

constexpr size_t kExtensionArgumentCount = 15;

// libsensor_extension uses its own selector table; these are not Android SensorType values or QSH
// sensor IDs. Values used by Nothing's SensorMonitorImpl for the screen-off-wakelock state follow.
constexpr int kProximityExtensionSelector = 4;
constexpr int kScreenOffWakelockCommand = 7;
constexpr int kWakelockAcquired = 1;
constexpr int kWakelockReleased = 2;

// Values used by Nothing's NothSensorHelper for display-state notifications. The Goodix ambient
// light and proximity functions share an SSC driver, which needs this notification to keep
// evaluating proximity after the panel is powered off.
constexpr int kAmbientLightExtensionSelector = 5;
constexpr int kCustomerCommand = 1;
constexpr int kSetDisplayState = 0x11174;
constexpr int kDisplayOff = 0;
constexpr int kDisplayOn = 1;

// panel_power_state uses the SDE_MODE_DPMS_* values from sde_drm.h.
constexpr int kPanelPowerOff = 5;
constexpr auto kFastPanelPollInterval = 10ms;
constexpr auto kSlowPanelPollInterval = 100ms;
constexpr auto kTransitionWakelockTimeout = 2s;

constexpr char kTransitionWakelockName[] = "SensorsHAL_ProximityPanelTransition";
constexpr const char* kPanelPowerStatePaths[] = {
        "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/"
        "sde-conn-2-DSI-1/panel_power_state",
        "/sys/class/drm/sde-conn-2-DSI-1/panel_power_state",
        "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/"
        "sde-conn-1-DSI-1/panel_power_state",
        "/sys/class/drm/sde-conn-1-DSI-1/panel_power_state",
};

class ProximitySensorExtension {
  public:
    static ProximitySensorExtension& getInstance() {
        static ProximitySensorExtension instance;
        return instance;
    }

    bool setSensorActive(int32_t sensorHandle, bool enabled) {
        std::lock_guard<std::mutex> activationLock(mActivationMutex);
        bool desiredState;
        bool updateExtension;

        {
            std::lock_guard<std::mutex> lock(mStateMutex);

            if (enabled) {
                mActiveSensorHandles.insert(sensorHandle);
            } else {
                mActiveSensorHandles.erase(sensorHandle);
                mNearSensorHandles.erase(sensorHandle);
            }

            desiredState = !mActiveSensorHandles.empty();
            updateExtension = desiredState != mProximityExtensionActive;
            ++mStateVersion;
        }
        mCondition.notify_one();

        if (!updateExtension) {
            return true;
        }

        const bool success = sendProximityState(desiredState);
        if (success) {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mProximityExtensionActive = desiredState;
        }
        return success;
    }

    void setSensorNear(int32_t sensorHandle, bool near) {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            if (mActiveSensorHandles.find(sensorHandle) == mActiveSensorHandles.end()) {
                return;
            }

            const bool changed = near ? mNearSensorHandles.insert(sensorHandle).second
                                      : mNearSensorHandles.erase(sensorHandle) != 0;
            if (!changed) {
                return;
            }

            // Keep the AP awake across the short NEAR-to-panel-off transition so the display-state
            // command reaches SSC before the display suspend blocker can be released.
            if (near || (mDisplayStateKnown && !mDisplayOn)) {
                holdTransitionWakelockLocked();
            } else {
                releaseTransitionWakelockLocked();
            }

            ++mStateVersion;
        }
        mCondition.notify_one();
    }

  private:
    ProximitySensorExtension() : mWorker(&ProximitySensorExtension::threadLoop, this) {}

    ~ProximitySensorExtension() {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mStop = true;
            ++mStateVersion;
        }
        mCondition.notify_one();
        mWorker.join();

        if (mPanelStateFd >= 0) {
            close(mPanelStateFd);
        }
    }

    ProximitySensorExtension(const ProximitySensorExtension&) = delete;
    ProximitySensorExtension& operator=(const ProximitySensorExtension&) = delete;

    bool shouldMonitorPanelLocked() const {
        if (!mDisplayStateKnown) {
            return !mActiveSensorHandles.empty();
        }
        return !mDisplayOn || (!mActiveSensorHandles.empty() && !mNearSensorHandles.empty());
    }

    void holdTransitionWakelockLocked() {
        if (!mTransitionWakelockHeld) {
            if (acquire_wake_lock(PARTIAL_WAKE_LOCK, kTransitionWakelockName) != 0) {
                ALOGE("Failed to acquire proximity panel-transition wakelock");
                return;
            }
            mTransitionWakelockHeld = true;
        }

        mTransitionWakelockDeadline = std::chrono::steady_clock::now() + kTransitionWakelockTimeout;
    }

    void releaseTransitionWakelockLocked() {
        if (!mTransitionWakelockHeld) {
            return;
        }

        release_wake_lock(kTransitionWakelockName);
        mTransitionWakelockHeld = false;
    }

    int readPanelPowerState() {
        if (mPanelStateFd < 0) {
            for (const char* path : kPanelPowerStatePaths) {
                mPanelStateFd = open(path, O_RDONLY | O_CLOEXEC);
                if (mPanelStateFd >= 0) {
                    ALOGI("Monitoring panel power state from %s", path);
                    break;
                }
            }

            if (mPanelStateFd < 0) {
                if (!mPanelOpenErrorLogged) {
                    ALOGE("Unable to open the panel_power_state kernel node");
                    mPanelOpenErrorLogged = true;
                }
                return -1;
            }
        }

        char value[16];
        const ssize_t bytesRead = pread(mPanelStateFd, value, sizeof(value) - 1, 0);
        if (bytesRead <= 0) {
            close(mPanelStateFd);
            mPanelStateFd = -1;
            return -1;
        }

        value[bytesRead] = '\0';
        return atoi(value);
    }

    bool executeCommand(int extensionSelector, int command,
                        std::array<int, kExtensionArgumentCount> arguments) {
        // The proprietary API does not define concurrent-client behavior; serialize commands from
        // the activation path and panel-state worker.
        std::lock_guard<std::mutex> commandLock(mCommandMutex);

        _noth_sensor_extension_ extension{};
        int status = noth_sensor_extension_create_client(&extension, extensionSelector);
        if (status != 0) {
            ALOGE("Failed to create sensor extension client for selector %d: %d",
                  extensionSelector, status);
            return false;
        }

        std::array<int, kExtensionArgumentCount> response = {};
        status = noth_sensor_extension_exec_sync_int_int(
                &extension, command, 0, arguments.data(), response.data());
        const int releaseStatus = noth_sensor_extension_release_client(&extension);

        if (status != 0) {
            ALOGE("Sensor extension command %d for selector %d failed: %d", command,
                  extensionSelector, status);
        }
        if (releaseStatus != 0) {
            ALOGW("Failed to release sensor extension client for selector %d: %d",
                  extensionSelector, releaseStatus);
        }

        return status == 0;
    }

    bool sendProximityState(bool active) {
        std::array<int, kExtensionArgumentCount> arguments = {};
        arguments[0] = active ? kWakelockAcquired : kWakelockReleased;

        const bool success =
                executeCommand(kProximityExtensionSelector, kScreenOffWakelockCommand, arguments);
        if (success) {
            ALOGI("Proximity screen-off wakelock state is %s", active ? "active" : "inactive");
        }
        return success;
    }

    bool sendDisplayState(bool displayOn) {
        std::array<int, kExtensionArgumentCount> arguments = {};
        arguments[0] = kSetDisplayState;
        arguments[1] = displayOn ? kDisplayOn : kDisplayOff;

        const bool success =
                executeCommand(kAmbientLightExtensionSelector, kCustomerCommand, arguments);
        if (success) {
            ALOGI("Goodix SSC display state is %s", displayOn ? "on" : "off");
        }
        return success;
    }

    void waitForStateChangeLocked(std::unique_lock<std::mutex>& lock,
                                  std::chrono::milliseconds timeout) {
        const uint64_t stateVersion = mStateVersion;
        mCondition.wait_for(lock, timeout, [this, stateVersion] {
            return mStop || mStateVersion != stateVersion;
        });
    }

    void threadLoop() {
        std::unique_lock<std::mutex> lock(mStateMutex);

        while (!mStop) {
            if (!shouldMonitorPanelLocked()) {
                mCondition.wait(lock, [this] { return mStop || shouldMonitorPanelLocked(); });
                continue;
            }

            lock.unlock();
            const int panelPowerState = readPanelPowerState();
            lock.lock();

            if (mStop) {
                break;
            }
            if (panelPowerState < 0) {
                waitForStateChangeLocked(lock, kSlowPanelPollInterval);
                continue;
            }

            const bool displayOn = panelPowerState != kPanelPowerOff;
            if (!mDisplayStateKnown || displayOn != mDisplayOn) {
                lock.unlock();
                const bool success = sendDisplayState(displayOn);
                lock.lock();

                if (success) {
                    mDisplayStateKnown = true;
                    mDisplayOn = displayOn;
                    releaseTransitionWakelockLocked();
                    ++mStateVersion;
                    continue;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            if (mTransitionWakelockHeld && now >= mTransitionWakelockDeadline) {
                ALOGW("Timed out waiting for a proximity panel transition");
                releaseTransitionWakelockLocked();
            }

            waitForStateChangeLocked(lock, mTransitionWakelockHeld ? kFastPanelPollInterval
                                                                   : kSlowPanelPollInterval);
        }

        releaseTransitionWakelockLocked();
    }

    std::mutex mActivationMutex;
    std::mutex mStateMutex;
    std::mutex mCommandMutex;
    std::condition_variable mCondition;
    std::unordered_set<int32_t> mActiveSensorHandles;
    std::unordered_set<int32_t> mNearSensorHandles;
    uint64_t mStateVersion = 0;
    bool mStop = false;
    bool mProximityExtensionActive = false;
    bool mDisplayStateKnown = false;
    bool mDisplayOn = true;
    bool mTransitionWakelockHeld = false;
    std::chrono::steady_clock::time_point mTransitionWakelockDeadline;
    int mPanelStateFd = -1;
    bool mPanelOpenErrorLogged = false;
    std::thread mWorker;
};

}  // anonymous namespace

bool notifyProximitySensorActivation(int32_t sensorHandle, bool enabled) {
    return ProximitySensorExtension::getInstance().setSensorActive(sensorHandle, enabled);
}

void notifyProximitySensorEvent(int32_t sensorHandle, bool near) {
    ProximitySensorExtension::getInstance().setSensorNear(sensorHandle, near);
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace sensors
}  // namespace hardware
}  // namespace android
