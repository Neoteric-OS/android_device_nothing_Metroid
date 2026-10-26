#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/nothing/Metroid',
    'hardware/qcom-caf/sm8750',
]

blob_fixups: blob_fixups_user_type = {
    'vendor/bin/init.kernel.post_boot-memory.sh': blob_fixup()
        .regex_replace('#configure_zram', 'configure_zram'),
    (
        'vendor/bin/hw/android.hardware.security.keymint-service-spu-qti',
        'vendor/lib64/libspukeymint.so',
    ): blob_fixup()
        .replace_needed(
            'android.hardware.security.sharedsecret-V2-ndk.so',
            'android.hardware.security.sharedsecret-V1-ndk.so',
        ),
    (
        'vendor/bin/hw/audiohalservice.qti',
        'vendor/bin/hw/vendor.qti.hardware.display.composer-service',
        'vendor/lib64/camera/components/com.arcsoft.node.motiondetect.so',
        'vendor/lib64/hw/camera.qcom.so',
        'vendor/lib64/hw/libaudioeffecthal.qti.so',
        'vendor/lib64/libcamerapoweroptfeature.so',
        'vendor/lib64/libcamxcoreutils.so',
        'vendor/lib64/libcamxods.so',
        'vendor/lib64/libsdmclient.so',
        'vendor/lib64/soundfx/libquasar.so',
    ): blob_fixup()
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),
    'vendor/etc/audio/sku_tuna/resourcemanager_tuna_qrd.xml': blob_fixup()
        .regex_replace('get_module_version="true"', 'get_module_version="false"')
        .regex_replace('<param concurrent_capture="false" />', '<param concurrent_capture="true" />'),
    'vendor/etc/camera/camxoverridesettings.txt': blob_fixup()
        .regex_replace('maxNonHfrFps=30', 'maxNonHfrFps=60'),
    'vendor/etc/init/hw/init.qcom.rc': blob_fixup()
        .regex_replace(r'[\t ]*interface vendor\.qti\.hardware\.wigig\.netperftuner@1\.0::INetPerfTuner default\r?\n', ''),
    'vendor/etc/init/qms.rc': blob_fixup()
        .regex_replace(
            r'(service\s+vendor\.qms\s+/vendor/bin/qms\s*\n)(?!\s*user root\b)',
            r'\1    user root\n',
        ),
    'vendor/etc/init/vendor.dpmd.rc': blob_fixup()
        .regex_replace(
            r'(service\s+vendor\.dpmd\s+/vendor/bin/vendor\.dpmd\s*\n)(?!\s*user root\b)',
            r'\1    user root\n',
        ),
    'vendor/etc/seccomp_policy/c2audio.vendor.ext-arm64.policy': blob_fixup()
        .add_line_if_missing('setsockopt: 1')
        .add_line_if_missing('lseek: 1'),
    'vendor/etc/seccomp_policy/gnss@2.0-qsap-location.policy': blob_fixup()
        .add_line_if_missing('sched_get_priority_min: 1')
        .add_line_if_missing('sched_get_priority_max: 1')
        .add_line_if_missing('lseek: 1'),
    (
        'vendor/etc/seccomp_policy/atfwd@2.0.policy',
        'vendor/etc/seccomp_policy/c2audio.vendor.ext-arm.policy',
        'vendor/etc/seccomp_policy/codec2.vendor.ext-arm64.policy',
        'vendor/etc/seccomp_policy/gnss@2.0-edgnss-daemon.policy',
        'vendor/etc/seccomp_policy/gnss@2.0-xtra-daemon.policy',
        'vendor/etc/seccomp_policy/qesdksec.policy',
        'vendor/etc/seccomp_policy/qti-systemd.policy',
        'vendor/etc/seccomp_policy/syshealthmon.policy',
    ): blob_fixup()
        .add_line_if_missing('lseek: 1'),
    (
        'vendor/lib64/libagm.so',
        'vendor/lib64/libar-pal.so',
        'vendor/lib64/libfmpal.so',
        'vendor/lib64/libhfp_pal.so',
        'vendor/lib64/libmcs.so',
    ): blob_fixup()
        .replace_needed('libaudioroute.so', 'libaudioroute-v34.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'Metroid',
    'nothing',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
