#
# Copyright (C) 2024 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/nothing/Metroid

# Configure updatable_apex.mk
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# Enforce generic ramdisk allow list
$(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)

# Enable project quotas and casefolding for emulated storage without sdcardfs
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

# Enable virtual AB with vendor ramdisk
$(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/launch_with_vendor_ramdisk.mk)

# Inherit from the proprietary files makefile.
$(call inherit-product, vendor/nothing/Metroid/Metroid-vendor.mk)

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-8192-dalvik-heap.mk)

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(DEVICE_PATH)

# AAPT
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# A/B
$(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/vabc_features.mk)

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_system=true \
    POSTINSTALL_PATH_system=system/bin/otapreopt_script \
    FILESYSTEM_TYPE_system=ext4 \
    POSTINSTALL_OPTIONAL_system=true

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_vendor=true \
    POSTINSTALL_PATH_vendor=bin/checkpoint_gc \
    FILESYSTEM_TYPE_vendor=ext4 \
    POSTINSTALL_OPTIONAL_vendor=true

PRODUCT_PACKAGES += \
    checkpoint_gc \
    otapreopt_script

# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio.common-V1-ndk \
    android.hardware.audio.common-V2-ndk \
    android.hardware.audio.common-V3-ndk \
    android.hardware.audio.core.sounddose-V1-ndk.vendor \
    android.hardware.audio.effect-V3-ndk.vendor \
    android.hardware.audio.core-V2-ndk.vendor \
    android.hardware.soundtrigger3-V1-ndk.vendor \
    vendor.qti.hardware.agm-V1-ndk \
    vendor.qti.hardware.paleventnotifier-V2-ndk.vendor

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/audio/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio/sku_tuna_qssi/audio_policy_volumes.xml

# Boot control
PRODUCT_PACKAGES += \
    android.hardware.boot-service.qti \
    android.hardware.boot-service.qti.recovery

PRODUCT_PACKAGES += \
    android.hardware.boot-V1-ndk.vendor

# Board
TARGET_BOARD_PLATFORM := sun

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := sun

# Camera
PRODUCT_PACKAGES += \
    android.frameworks.cameraservice.device-V2-ndk \
    android.frameworks.cameraservice.service-V2-ndk \
    android.hardware.camera.common-V1-ndk.vendor \
    android.hardware.camera.device-V1-ndk.vendor \
    android.hardware.camera.device-V2-ndk.vendor \
    android.hardware.camera.metadata-V1-ndk.vendor \
    android.hardware.camera.metadata-V2-ndk.vendor \
    android.hardware.camera.provider-V1-ndk.vendor \
    android.hardware.camera.provider-V2-ndk.vendor \
    android.hardware.common-V2-ndk_platform.vendor \
    vendor.qti.hardware.camera.aon-V1-ndk.vendor \
    vendor.qti.hardware.camera.aon-V2-ndk.vendor \
    vendor.qti.hardware.camera.offlinecamera-V1-ndk.vendor \
    vendor.qti.hardware.camera.offlinecamera-V2-ndk.vendor \
    vendor.qti.hardware.camera.postproc@1.0.vendor

PRODUCT_PACKAGES += \
    camera.device@1.0-impl \
    libcamera2ndk_vendor \
    libcamera_metadata.vendor \
    libcameraservice \
    libexif.vendor \
    libpiex.vendor \
    libutilscallstack.vendor \
    libyuv.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.concurrent.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.concurrent.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml

# Cas
PRODUCT_PACKAGES += \
    android.hardware.cas-V1-ndk.vendor

# ConfigStore
PRODUCT_PACKAGES += \
    vendor.qti.hardware.capabilityconfigstore@1.0.vendor

# Contexthub
PRODUCT_PACKAGES += \
    android.hardware.contexthub-V2-ndk.vendor \
    android.hardware.contexthub-V3-ndk.vendor \
    com.android.hardware.contexthub

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.context_hub.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.context_hub.xml

# Display
PRODUCT_PACKAGES += \
    libgui_vendor \
    vendor.display.config@1.1 \
    vendor.display.config@1.11.vendor \
    vendor.display.config@2.0 \
    vendor.qti.hardware.display.mapper@1.0.vendor \
    vendor.qti.hardware.display.mapper@1.1.vendor \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.3.vendor

PRODUCT_PACKAGES += \
    vendor.qti.hardware.display.composer3-V1-ndk.vendor \
    vendor.qti.hardware.display.composer3-V2-ndk.vendor \
    vendor.qti.hardware.display.composer3-V3-ndk.vendor \
    vendor.qti.hardware.display.config-V1-ndk.vendor \
    vendor.qti.hardware.display.config-V2-ndk.vendor \
    vendor.qti.hardware.display.config-V3-ndk.vendor \
    vendor.qti.hardware.display.config-V4-ndk.vendor \
    vendor.qti.hardware.display.config-V5-ndk.vendor \
    vendor.qti.hardware.display.config-V6-ndk.vendor \
    vendor.qti.hardware.display.config-V7-ndk.vendor \
    vendor.qti.hardware.display.config-V8-ndk.vendor \
    vendor.qti.hardware.display.config-V9-ndk.vendor \
    vendor.qti.hardware.display.config-V10-ndk.vendor \
    vendor.qti.hardware.display.config-V11-ndk.vendor \
    vendor.qti.hardware.display.config-V12-ndk.vendor \
    vendor.qti.hardware.display.config-V13-ndk.vendor \
    vendor.qti.hardware.display.demura-V1-ndk.vendor \
    vendor.qti.hardware.display.postproc-V1-ndk.vendor

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm-service.clearkey

PRODUCT_PACKAGES += \
    android.hardware.drm-V1-ndk.vendor

PRODUCT_PACKAGES += \
    libdrm.vendor

# DumpState
PRODUCT_PACKAGES += \
    android.hardware.dumpstate-V1-ndk.vendor

# ELF
PRODUCT_CHECK_PREBUILT_MAX_PAGE_SIZE := false

# Fastbootd
PRODUCT_PACKAGES += \
    fastbootd

# Fingerprint
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml

PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint-service.nothing

# Gatekeeper
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0.vendor \
    android.hardware.gatekeeper-V1-ndk.vendor \
    libgatekeeper.vendor

# Graphics
PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator-V1-ndk.vendor \
    android.hardware.graphics.allocator-V2-ndk.vendor \
    android.hardware.graphics.common-V4-ndk.vendor \
    android.hardware.graphics.common-V5-ndk.vendor \
    android.hardware.graphics.common-V6-ndk.vendor \
    android.hardware.graphics.composer3-V2-ndk.vendor \
    android.hardware.graphics.composer3-V3-ndk.vendor \
    android.hardware.graphics.composer3-V4-ndk.vendor \
    android.hardware.graphics.composer@2.1.vendor \
    android.hardware.graphics.composer@2.2.vendor \
    android.hardware.graphics.composer@2.3.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute-0.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level-1.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_1.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_3.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_3.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2023-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2023-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml

# GNSS
PRODUCT_PACKAGES += \
    android.frameworks.location.altitude-V2-ndk.vendor \
    android.hardware.gnss-V3-ndk.vendor \
    android.hardware.gnss-V4-ndk.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.location.gps.xml

# Health
PRODUCT_PACKAGES += \
    android.hardware.health-service.qti \
    android.hardware.health-service.qti_recovery

PRODUCT_PACKAGES += \
    android.hardware.health@1.0.vendor \
    android.hardware.health@2.1.vendor

PRODUCT_PACKAGES += \
    android.hardware.health-V1-ndk.vendor \
    android.hardware.health-V2-ndk.vendor \
    android.hardware.health-V3-ndk.vendor

# HWUI
TARGET_USES_VULKAN := true

# IPACM
PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml \
    IPACM_Filter_cfg.xml

# Identity
PRODUCT_PACKAGES += \
    android.hardware.identity-V5-ndk.vendor

# Init
PRODUCT_PACKAGES += \
    fstab.qcom \
    init.Metroid.rc \
    init.recovery.qcom.rc \
    init.target.rc \
    ueventd.odm.rc \
    ueventd.qcom.rc

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/init/fstab.qcom:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom

# Keymint
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0.vendor \
    android.hardware.keymaster@4.0.vendor \
    android.hardware.keymaster@4.1.vendor

PRODUCT_PACKAGES += \
    android.hardware.authsecret@1.0.vendor \
    android.hardware.authsecret-V1-ndk.vendor \
    android.hardware.keymaster-V3-ndk.vendor \
    android.hardware.keymaster-V4-ndk.vendor \
    android.hardware.keymaster@3.0.vendor \
    android.hardware.keymaster@4.1.vendor \
    android.hardware.security.keymint-V1-ndk.vendor \
    android.hardware.security.keymint-V2-ndk.vendor \
    android.hardware.security.keymint-V3-ndk.vendor \
    android.hardware.security.rkp-V3-ndk.vendor \
    android.hardware.security.secureclock-V1-ndk.vendor \
    android.hardware.security.sharedsecret-V1-ndk.vendor \
    android.system.keystore2-V1-ndk.vendor \
    lib_android_keymaster_keymint_utils.vendor \
    libsoft_attestation_cert.vendor \
    libkeymaster_messages.vendor \
    libkeymaster_portable.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.keystore.app_attest_key.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.keystore.app_attest_key.xml \
    frameworks/native/data/etc/android.software.device_id_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.device_id_attestation.xml

PRODUCT_PACKAGES += \
    android.hardware.hardware_keystore_V3.xml

# Media
PRODUCT_PACKAGES += \
    android.hardware.media.c2@1.2.vendor \
    libOmxCore \
    libcodec2_aidl.vendor \
    libcodec2_hidl@1.0.vendor \
    libcodec2_hidl@1.1.vendor \
    libcodec2_hidl@1.2.vendor \
    libcodec2_soft_common.vendor \
    libcodec2_vndk.vendor \
    libmm-omxcore \
    libstagefright_softomx_plugin.vendor \
    libstagefright_softomx.vendor \
    libsfplugin_ccodec_utils.vendor \
    libstagefrighthw

PRODUCT_PACKAGES += \
    android.hardware.media.bufferpool2-V1-ndk.vendor \
    android.hardware.media.bufferpool2-V2-ndk.vendor \
    android.hardware.media.c2-V1-ndk.vendor

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml

# Memtrack
PRODUCT_PACKAGES += \
    vendor.qti.hardware.memtrack-service

# Network
PRODUCT_PACKAGES += \
    android.system.net.netd-V1-ndk.vendor

# NDK
NEED_AIDL_NDK_PLATFORM_BACKEND := true

# NFC
PRODUCT_PACKAGES += \
    android.hardware.secure_element-V1-ndk.vendor \
    android.hardware.weaver-V2-ndk \
    android.se.omapi-V1-ndk

PRODUCT_PACKAGES += \
    android.hardware.nfc-service.st

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.nfc.ese.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml

# Overlays
PRODUCT_PACKAGES += \
    ApertureRes \
    aptxalsOverlay \
    FrameworksRes \
    SettingsRes \
    SettingsProviderRes \
    SystemUIRes \
    TelephonyRes \
    WifiRes \
    WifiResTarget \
    WifiResTarget_spf

# Partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true
PRODUCT_PACKAGES += \
    vendor_bt_firmware_mountpoint \
    vendor_dsp_mountpoint \
    vendor_firmware_mnt_mountpoint

# Power
PRODUCT_PACKAGES += \
    android.hardware.power-service-qti

PRODUCT_PACKAGES += \
    android.hardware.power@1.2.vendor

PRODUCT_PACKAGES += \
    android.hardware.power-V4-ndk.vendor \
    android.hardware.power-V5-ndk.vendor

PRODUCT_COPY_FILES += \
    vendor/qcom/opensource/power/config/sun/powerhint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.xml

# QSPA
PRODUCT_PACKAGES += \
    vendor.qti.qspa-service

# QMI
PRODUCT_PACKAGES += \
    libcurl.vendor \
    libjsoncpp.vendor \
    libsqlite.vendor

# QTI Components
TARGET_COMMON_QTI_COMPONENTS := \
    alarm \
    audio \
    bt \
    display \
    gps \
    overlay \
    perf \
    telephony \
    usb \
    wfd \
    wlan

TARGET_USE_AIDL_QTI_BT_AUDIO := true

# Radio
PRODUCT_PACKAGES += \
    libandroid_runtime_lazy.vendor \
    libprotobuf-cpp-full \
    libnetutils.vendor \
    librmnetctl

# RenderScript
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Servicetracker
PRODUCT_PACKAGES += \
    vendor.qti.hardware.servicetracker@1.2.vendor

PRODUCT_PACKAGES += \
    vendor.qti.hardware.servicetrackeraidl-V1-ndk.vendor

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors-service.multihal

PRODUCT_PACKAGES += \
    android.hardware.sensors-V2-ndk.vendor

PRODUCT_PACKAGES += \
    libsensorndkbridge \
    sensors.dynamic_sensor_hal \
    android.frameworks.sensorservice-V1-ndk.vendor \
    android.frameworks.sensorservice@1.0.vendor

PRODUCT_PACKAGES += \
    sensors.nothing

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.dynamic.head_tracker.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.dynamic.head_tracker.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml

# Shims
PRODUCT_PACKAGES += \
    libaudioroute-v34 \
    libtinyxml2-v34 \
    libprocessgroup.vendor \
    libaudioutils_shim

# Shipping API level
BOARD_SHIPPING_API_LEVEL := 202404
PRODUCT_SHIPPING_API_LEVEL := 35

# Touchscreen
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

# Thermal
PRODUCT_PACKAGES += \
    android.hardware.thermal-V2-ndk.vendor

# Trusted User Interface (TUI)
PRODUCT_PACKAGES += \
    vendor.qti.hardware.systemhelper@1.0.vendor

# Update engine
PRODUCT_PACKAGES += \
    update_engine \
    update_engine_sideload \
    update_verifier

# Vendor service manager
PRODUCT_PACKAGES += \
    vndservice \
    vndservicemanager

# Verified boot
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.verified_boot.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.verified_boot.xml

# WiFi
PRODUCT_PACKAGES += \
    libcld80211 \
    libkeystore-engine-wifi-hidl \
    libkeystore-wifi-hidl \
    libpasn \
    libwifi-hal \
    libwifi-hal-ctrl \
    libwpa_client \
    wpa_cli

PRODUCT_PACKAGES += \
    android.hardware.wifi-V1-ndk.vendor \
    android.hardware.wifi-V2-ndk.vendor \
    android.hardware.wifi.hostapd-V2-ndk.vendor \
    android.hardware.wifi.supplicant-V2-ndk.vendor \
    android.hardware.wifi.supplicant-V3-ndk.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml

# WiFi firmware symlinks
PRODUCT_PACKAGES += \
    firmware_wlanmdsp.otaupdate_symlink \
    firmware_WCNSS_qcom_cfg.ini_symlink-wcn7750 \
    firmware_wlan_mac.bin_symlink-wcn7750

# WiFi Display
PRODUCT_PACKAGES += \
    android.media.audio.common.types-V2-cpp \
    libdisplayconfig.system.qti \
    qti-audio-types-aidl-V1-ndk.vendor
