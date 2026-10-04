/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Vibrator.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#include <algorithm>

#include <cmath>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

// Constans
static constexpr const char* kVibratorSysfsRoot = "/sys/class/leds/vibrator_nt/";
static constexpr const char* kNodeActivate      = "activate";
static constexpr const char* kNodeBrightness    = "brightness";
static constexpr const char* kNodeLoop          = "loop";
static constexpr const char* kNodeDuration      = "duration";
static constexpr const char* kNodeIndex         = "index";
static constexpr const char* kNodeGain          = "gain";
static constexpr const char* kNodeVmax          = "vmax";

// Default boost voltage
static constexpr uint32_t kDefaultVmaxMv = 9000;

// Default gain (AW8693x default 0x80 = 50%)
static constexpr uint8_t kDefaultGain = 0x80;

// EffectStrength → gain byte
static constexpr uint8_t kGainLight  = 0x40;  // 25%
static constexpr uint8_t kGainMedium = 0x80;  // 50%
static constexpr uint8_t kGainStrong = 0xFF;  // 100%

// Max duration for keyboard effects
static constexpr int32_t kShortDurationMs = 100;

// Time between the start of the two clicks in DOUBLE_CLICK.
static constexpr int32_t kDoubleClickGapMs = 100;

// Composition limits
static constexpr int32_t kComposeSizeMax         = 16;
static constexpr int32_t kComposeDelayMaxMs      = 1000;

// ---------------------------------------------------------------------------
// RAM waveform effect table
// (index into haptic_ram.bin, duration in ms)
// ---------------------------------------------------------------------------
struct RamEffect { int index; int durationMs; };
static constexpr RamEffect kEffectMap[] = {
    /* CLICK */        {1,  20},
    /* DOUBLE_CLICK */ {2,  50},
    /* TICK */         {1,   8},
    /* THUD */         {4,  60},
    /* POP */          {1,  20},
    /* HEAVY_CLICK */  {2,  35},
};

struct PrimInfo { int index; int durationMs; };
static PrimInfo getPrimInfo(CompositePrimitive p) {
    switch (p) {
        case CompositePrimitive::CLICK:      return {1, 20};
        case CompositePrimitive::THUD:       return {4, 60};
        case CompositePrimitive::LIGHT_TICK: return {1,  8};
        case CompositePrimitive::LOW_TICK:   return {2, 10};
        default:                             return {0,  0};  // NOOP / unsupported
    }
}

Vibrator::Vibrator() {
    LOG(INFO) << "Vibrator HAL: init";

    writeNode(kNodeVmax, std::to_string(kDefaultVmaxMv));
    setGainLocked(kDefaultGain);
}

Vibrator::~Vibrator() {
    {
        std::lock_guard lock(mMutex);
        mIsVibrating = false;
        mVibrationCallback = nullptr;
    }
    writeNode(kNodeActivate, "0");
}

bool Vibrator::writeNode(const std::string& node, const std::string& value) {
    const std::string path = std::string(kVibratorSysfsRoot) + node;
    if (!::android::base::WriteStringToFile(value, path)) {
        LOG(ERROR) << "Vibrator HAL: write '" << value << "' -> " << path
                   << " failed: " << strerror(errno);
        return false;
    }
    return true;
}

std::string Vibrator::readNode(const std::string& node) {
    std::string buf;
    if (!::android::base::ReadFileToString(std::string(kVibratorSysfsRoot) + node, &buf)) {
        return "";
    }
    return ::android::base::Trim(buf);
}

void Vibrator::setGainLocked(uint8_t gain) {
    writeNode(kNodeGain, std::to_string(static_cast<unsigned>(gain)));
}

uint8_t Vibrator::strengthToGain(EffectStrength s) {
    switch (s) {
        case EffectStrength::LIGHT:  return kGainLight;
        case EffectStrength::STRONG: return kGainStrong;
        default:                     return kGainMedium;
    }
}

void Vibrator::stopLocked() {
    writeNode(kNodeActivate, "0");
}

void Vibrator::playHaptics(int ramIndex, int32_t durationMs, uint8_t gainByte) {
    writeNode(kNodeDuration,     std::to_string(durationMs));
    if (ramIndex <= 0) {
        if (durationMs <= kShortDurationMs) {
            // Short tap: single non-looping click.
            writeNode(kNodeIndex,        "1");
        } else {
            // Long vibrations
            writeNode(kNodeIndex,        "4");
        }
    } else {
        writeNode(kNodeIndex,        std::to_string(ramIndex));
    }
    if (ramIndex != 0 || durationMs <= kShortDurationMs) {
        writeNode(kNodeLoop,         "0 0");
    }
    setGainLocked(gainByte);
    if (ramIndex != 0 || durationMs <= kShortDurationMs) {
        writeNode(kNodeBrightness,   "1");
    } else {
        writeNode(kNodeActivate,   "1");
    }
}

void Vibrator::dispatchVibrate(int32_t timeoutMs,
                               const std::shared_ptr<IVibratorCallback>& callback) {
    {
        std::lock_guard lock(mMutex);
        if (mIsVibrating) {
            return;
        }
        mVibrationCallback = std::shared_ptr<IVibratorCallback>(callback);
        mIsVibrating = true;
    }
    // Note that thread lambdas aren't using implicit capture [=], to avoid capturing "this",
    // which may be asynchronously destructed.
    std::thread([timeoutMs, callback, sharedThis = this->ref<Vibrator>()] {
        LOG(VERBOSE) << "Starting delayed callback on another thread";
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));

        if (sharedThis) {
            std::shared_ptr<IVibratorCallback> vibrationCallback;
            {
                std::lock_guard lock(sharedThis->mMutex);
                sharedThis->mIsVibrating = false;
                if (sharedThis->mVibrationCallback == callback) {
                    vibrationCallback = std::move(sharedThis->mVibrationCallback);
                    sharedThis->mVibrationCallback = nullptr;
                }
            }
            if (vibrationCallback) {
                LOG(VERBOSE) << "Notifying callback onComplete";
                if (!vibrationCallback->onComplete().isOk()) {
                    LOG(ERROR) << "Failed to call onComplete";
                }
            }
        }
    }).detach();
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    int32_t capabilities = IVibrator::CAP_ON_CALLBACK |
        IVibrator::CAP_PERFORM_CALLBACK |
        IVibrator::CAP_AMPLITUDE_CONTROL |
        IVibrator::CAP_COMPOSE_EFFECTS;
    *_aidl_return = capabilities;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    LOG(VERBOSE) << "Vibrator HAL: off()";
    std::shared_ptr<IVibratorCallback> callback;
    {
        std::lock_guard lock(mMutex);
        callback = std::move(mVibrationCallback);
        mIsVibrating = false;
        mVibrationCallback = nullptr;
    }
    stopLocked();
    if (callback) {
        std::thread([callback] {
            LOG(VERBOSE) << "Notifying callback onComplete";
            if (!callback->onComplete().isOk()) {
                LOG(ERROR) << "Failed to call onComplete";
            }
        }).detach();
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    LOG(VERBOSE) << "Vibrator HAL: on(" << timeoutMs << " ms)";
    if (timeoutMs <= 0) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }

    uint8_t gain;
    {
        std::lock_guard lock(mMutex);
        gain = static_cast<uint8_t>(
                std::clamp(static_cast<int>(mCurrentAmplitude * 0xFF), 1, 0xFF));
    }

    playHaptics(0, timeoutMs, gain);
    dispatchVibrate(timeoutMs, callback);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength strength,
                                     const std::shared_ptr<IVibratorCallback>& callback,
                                     int32_t* _aidl_return) {
    LOG(VERBOSE) << "Vibrator HAL: perform(" << static_cast<int>(effect) << ")";

    static const std::vector<Effect> kSupported = {
        Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK,
        Effect::THUD, Effect::POP, Effect::HEAVY_CLICK,
    };
    if (std::find(kSupported.begin(), kSupported.end(), effect) == kSupported.end()) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }
    if (strength != EffectStrength::LIGHT && strength != EffectStrength::MEDIUM &&
        strength != EffectStrength::STRONG) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    const uint8_t gain = strengthToGain(strength);
    const auto& e = kEffectMap[static_cast<int>(effect)];

    if (effect == Effect::DOUBLE_CLICK) {
        const int32_t totalMs = kDoubleClickGapMs + e.durationMs;

        playHaptics(e.index, e.durationMs, gain);
        dispatchVibrate(totalMs, callback);

        usleep(kDoubleClickGapMs * 1000);

        playHaptics(e.index, e.durationMs, gain);

        *_aidl_return = totalMs;
        return ndk::ScopedAStatus::ok();
    }

    playHaptics(e.index, e.durationMs, gain);

    *_aidl_return = e.durationMs;
    dispatchVibrate(e.durationMs, callback);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::performVendorEffect(
        const VendorEffect& effect,
        const std::shared_ptr<IVibratorCallback>& callback) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {
        Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK,
        Effect::THUD, Effect::POP, Effect::HEAVY_CLICK,
    };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    if (amplitude <= 0.0f || amplitude > 1.0f) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }
    std::lock_guard<std::mutex> lock(mMutex);
    mCurrentAmplitude = amplitude;
    setGainLocked(static_cast<uint8_t>(
            std::clamp(static_cast<int>(amplitude * 0xFF), 1, 0xFF)));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* maxDelayMs) {
    *maxDelayMs = kComposeDelayMaxMs;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* maxSize) {
    *maxSize = kComposeSizeMax;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* supported) {
    *supported = {
        CompositePrimitive::NOOP,
        CompositePrimitive::CLICK,
        CompositePrimitive::THUD,
        CompositePrimitive::LIGHT_TICK,
        CompositePrimitive::LOW_TICK,
    };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive primitive,
                                                  int32_t* durationMs) {
    std::vector<CompositePrimitive> supported;
    getSupportedPrimitives(&supported);
    if (std::find(supported.begin(), supported.end(), primitive) == supported.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    *durationMs = getPrimInfo(primitive).durationMs;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& composite,
                                     const std::shared_ptr<IVibratorCallback>& callback) {
    if (composite.size() > static_cast<size_t>(kComposeSizeMax)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    std::vector<CompositePrimitive> supported;
    getSupportedPrimitives(&supported);

    int32_t totalMs = 0;
    for (auto& e : composite) {
        if (e.delayMs > kComposeDelayMaxMs) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        if (e.scale < 0.0f || e.scale > 1.0f) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        if (std::find(supported.begin(), supported.end(), e.primitive) == supported.end()) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        }
        int32_t d = 0;
        getPrimitiveDuration(e.primitive, &d);
        totalMs += e.delayMs + d;
    }

    // Pick the first non-NOOP primitive as representative waveform
    int firstIdx = 1;
    uint8_t firstGain = kGainMedium;
    for (auto& e : composite) {
        if (e.primitive != CompositePrimitive::NOOP) {
            firstIdx  = getPrimInfo(e.primitive).index;
            firstGain = static_cast<uint8_t>(
                    std::clamp(static_cast<int>(e.scale * kGainStrong), 1, 0xFF));
            break;
        }
    }
    playHaptics(firstIdx, totalMs, firstGain);
    dispatchVibrate(totalMs, callback);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id, Effect effect, EffectStrength strength) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t id) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getResonantFrequency(float* resonantFreqHz) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getQFactor(float* qFactor) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyResolution(float* freqResolutionHz) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyMinimum(float* freqMinimumHz) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getBandwidthAmplitudeMap(std::vector<float>* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwlePrimitiveDurationMax(int32_t* durationMs) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleCompositionSizeMax(int32_t* maxSize) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedBraking(std::vector<Braking>* supported) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::composePwle(const std::vector<PrimitivePwle>& composite,
                                         const std::shared_ptr<IVibratorCallback>& callback) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyToOutputAccelerationMap(
        std::vector<FrequencyAccelerationMapEntry>* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleV2PrimitiveDurationMaxMillis(int32_t* maxDurationMs) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleV2PrimitiveDurationMinMillis(int32_t* minDurationMs) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleV2CompositionSizeMax(int32_t* maxSize) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::composePwleV2(const CompositePwleV2& composite,
                                           const std::shared_ptr<IVibratorCallback>& callback) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
