#
# Copyright (C) 2026 Neoteric OS
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit_only.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from Metroid device
$(call inherit-product, device/nothing/Metroid/device.mk)

# Inherit from the Neoteric configuration.
$(call inherit-product, vendor/neoteric/target/product/neoteric-target.mk)

# Device identifier
PRODUCT_DEVICE := Metroid
PRODUCT_NAME := Metroid
PRODUCT_BRAND := Nothing
PRODUCT_MODEL := A024
PRODUCT_MANUFACTURER := Nothing

# Declare updates support
IS_OFFICIAL := true

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="Metroid-user 15 AQ3A.250728.001 2604141846 release-keys" \
    BuildFingerprint=Nothing/Metroid/Metroid:15/AQ3A.250728.001/2604141846:user/release-keys\
    DeviceName=Metroid \
    DeviceProduct=Metroid \
    SystemDevice=Metroid \
    SystemName=Metroid

PRODUCT_GMS_CLIENTID_BASE := android-nothing
