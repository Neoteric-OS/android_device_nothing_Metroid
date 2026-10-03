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

#pragma once

#include <aidl/android/hardware/vibrator/BnVibrator.h>
#include <android-base/thread_annotations.h>


#include <mutex>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

class Vibrator : public BnVibrator {
  public:
    Vibrator();
    ~Vibrator();

    ndk::ScopedAStatus getCapabilities(int32_t* _aidl_return) override;
    ndk::ScopedAStatus off() override;
    ndk::ScopedAStatus on(int32_t timeoutMs,
                          const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus perform(Effect effect, EffectStrength strength,
                               const std::shared_ptr<IVibratorCallback>& callback,
                               int32_t* _aidl_return) override;
    ndk::ScopedAStatus performVendorEffect(
            const VendorEffect& effect,
            const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus getSupportedEffects(std::vector<Effect>* _aidl_return) override;
    ndk::ScopedAStatus setAmplitude(float amplitude) override;
    ndk::ScopedAStatus setExternalControl(bool enabled) override;
    ndk::ScopedAStatus getCompositionDelayMax(int32_t* maxDelayMs) override;
    ndk::ScopedAStatus getCompositionSizeMax(int32_t* maxSize) override;
    ndk::ScopedAStatus getSupportedPrimitives(std::vector<CompositePrimitive>* supported) override;
    ndk::ScopedAStatus getPrimitiveDuration(CompositePrimitive primitive,
                                            int32_t* durationMs) override;
    ndk::ScopedAStatus compose(const std::vector<CompositeEffect>& composite,
                               const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return) override;
    ndk::ScopedAStatus alwaysOnEnable(int32_t id, Effect effect,
                                      EffectStrength strength) override;
    ndk::ScopedAStatus alwaysOnDisable(int32_t id) override;
    ndk::ScopedAStatus getResonantFrequency(float* resonantFreqHz) override;
    ndk::ScopedAStatus getQFactor(float* qFactor) override;
    ndk::ScopedAStatus getFrequencyResolution(float* freqResolutionHz) override;
    ndk::ScopedAStatus getFrequencyMinimum(float* freqMinimumHz) override;
    ndk::ScopedAStatus getBandwidthAmplitudeMap(std::vector<float>* _aidl_return) override;
    ndk::ScopedAStatus getPwlePrimitiveDurationMax(int32_t* durationMs) override;
    ndk::ScopedAStatus getPwleCompositionSizeMax(int32_t* maxSize) override;
    ndk::ScopedAStatus getSupportedBraking(std::vector<Braking>* supported) override;
    ndk::ScopedAStatus composePwle(const std::vector<PrimitivePwle>& composite,
                                   const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus getFrequencyToOutputAccelerationMap(
            std::vector<FrequencyAccelerationMapEntry>* _aidl_return) override;
    ndk::ScopedAStatus getPwleV2PrimitiveDurationMaxMillis(int32_t* maxDurationMs) override;
    ndk::ScopedAStatus getPwleV2PrimitiveDurationMinMillis(int32_t* minDurationMs) override;
    ndk::ScopedAStatus getPwleV2CompositionSizeMax(int32_t* maxSize) override;
    ndk::ScopedAStatus composePwleV2(const CompositePwleV2& composite,
                                     const std::shared_ptr<IVibratorCallback>& callback) override;

  private:
    mutable std::mutex mMutex;
    bool mIsVibrating GUARDED_BY(mMutex) = false;
    std::shared_ptr<IVibratorCallback> mVibrationCallback GUARDED_BY(mMutex) = nullptr;
    float mCurrentAmplitude GUARDED_BY(mMutex) = 1.0f;

    bool writeNode(const std::string& node, const std::string& value);
    std::string readNode(const std::string& node);

    void stopLocked();
    void playHaptics(int ramIndex, int32_t durationMs, uint8_t gainByte);
    void dispatchVibrate(int32_t timeoutMs,
                         const std::shared_ptr<IVibratorCallback>& callback);
    void setGainLocked(uint8_t gain);
    static uint8_t strengthToGain(EffectStrength strength);
    float readF0();
};

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
