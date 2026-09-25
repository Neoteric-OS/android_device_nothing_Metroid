#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE=Metroid
VENDOR=nothing

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
    -n | --no-cleanup)
        CLEAN_VENDOR=false
        ;;
    -k | --kang)
        KANG="--kang"
        ;;
    -s | --section)
        SECTION="${2}"
        shift
        CLEAN_VENDOR=false
        ;;
    *)
        SRC="${1}"
        ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        vendor/bin/hw/android.hardware.security.keymint-service-spu-qti | \
        vendor/lib64/libspukeymint.so)
            "${PATCHELF}" --replace-needed \
                "android.hardware.security.sharedsecret-V2-ndk.so" \
                "android.hardware.security.sharedsecret-V1-ndk.so" \
                "${2}"
            ;;
        vendor/lib64/libcamxcoreutils.so | \
		vendor/lib64/hw/camera.qcom.so | \
		vendor/lib64/hw/libaudioeffecthal.qti.so | \
		vendor/lib64/camera/components/com.arcsoft.node.motiondetect.so | \
		vendor/lib64/soundfx/libquasar.so | \
		vendor/lib64/libsdmclient.so | \
		vendor/lib64/libcamxods.so | \
		vendor/lib64/libcamerapoweroptfeature.so | \
		vendor/bin/hw/vendor.qti.hardware.display.composer-service | \
		vendor/bin/hw/audiohalservice.qti)
            "${PATCHELF}" --replace-needed \
                "libtinyxml2.so" \
                "libtinyxml2-v34.so" \
                "${2}"
            ;;
        vendor/etc/init/qms.rc)
            sed -i \
                's|\(service[[:space:]]\+vendor\.qms[[:space:]]\+/vendor/bin/qms[[:space:]]*$\)|\1\n    user root|' \
                "${2}"
            ;;
        vendor/etc/init/vendor.dpmd.rc)
            sed -i \
                's|\(service[[:space:]]\+vendor\.dpmd[[:space:]]\+/vendor/bin/vendor\.dpmd[[:space:]]*$\)|\1\n    user root|' \
                "${2}"
            ;;
        vendor/etc/init/hw/init.qcom.rc)
            sed -i '/interface vendor\.qti\.hardware\.wigig\.netperftuner@1\.0::INetPerfTuner default/d' "${2}"
            ;;
        vendor/etc/seccomp_policy/c2audio.vendor.ext-arm64.policy)
            [ "$2" = "" ] && return 0
            [ -n "$(tail -c 1 "${2}")" ] && echo >> "${2}"
            grep -q "setsockopt: 1" "${2}" || echo "setsockopt: 1" >> "${2}"
            grep -q "^lseek:" "${2}" || echo "lseek: 1" >> "${2}"
            ;;
        vendor/etc/seccomp_policy/gnss@2.0-qsap-location.policy)
            [ "$2" = "" ] && return 0
            [ -n "$(tail -c 1 "${2}")" ] && echo >> "${2}"
            grep -q "sched_get_priority_min: 1" "${2}" || echo "sched_get_priority_min: 1" >> "${2}"
            grep -q "sched_get_priority_max: 1" "${2}" || echo "sched_get_priority_max: 1" >> "${2}"
            grep -q "^lseek:" "${2}" || echo "lseek: 1" >> "${2}"
            ;;
        vendor/etc/seccomp_policy/atfwd@2.0.policy | \
        vendor/etc/seccomp_policy/c2audio.vendor.ext-arm.policy | \
        vendor/etc/seccomp_policy/codec2.vendor.ext-arm64.policy | \
        vendor/etc/seccomp_policy/gnss@2.0-edgnss-daemon.policy | \
        vendor/etc/seccomp_policy/gnss@2.0-xtra-daemon.policy | \
        vendor/etc/seccomp_policy/qesdksec.policy | \
        vendor/etc/seccomp_policy/qti-systemd.policy | \
        vendor/etc/seccomp_policy/syshealthmon.policy)
            [ "$2" = "" ] && return 0
            [ -n "$(tail -c 1 "${2}")" ] && echo >> "${2}"
            grep -q "^lseek:" "${2}" || echo "lseek: 1" >> "${2}"
            ;;
        vendor/lib64/libhfp_pal.so | \
		vendor/lib64/libfmpal.so | \
		vendor/lib64/libmcs.so | \
		vendor/lib64/libagm.so | \
		vendor/lib64/libar-pal.so)
            "${PATCHELF}" --replace-needed "libaudioroute.so" "libaudioroute-v34.so" "${2}"
            ;;
        vendor/etc/audio/sku_tuna/resourcemanager_tuna_qrd.xml)
            [ "$2" = "" ] && return 0
            sed -i 's/get_module_version="true"/get_module_version="false"/g' "${2}"
            ;;
    esac
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
