/*
 * Copyright (C) 2026 Neoteric OS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace android {
namespace hardware {
namespace sensors {
namespace V2_1 {
namespace implementation {

bool notifyProximitySensorActivation(int32_t sensorHandle, bool enabled);
void notifyProximitySensorEvent(int32_t sensorHandle, bool near);

}  // namespace implementation
}  // namespace V2_1
}  // namespace sensors
}  // namespace hardware
}  // namespace android
