#
# Copyright (C) 2007 The Android Open Source Project
# Copyright (C) 2012 The Cyanogenmod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# BoardConfig.mk
#
# Product-specific compile-time definitions.
#

BUILD_NETD := false

# inherit from the proprietary version
-include vendor/bn/encore/BoardConfigVendor.mk

TARGET_ARCH := arm
TARGET_BOARD_PLATFORM := omap3
TARGET_CPU_ABI := armeabi-v7a
ARCH_ARM_HAVE_ARMV7A := true
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a8
TARGET_GLOBAL_CFLAGS += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
# -fmodulo-sched -fmodulo-sched-allow-regmoves
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
# -fmodulo-sched -fmodulo-sched-allow-regmoves
TARGET_arm_CFLAGS := -Os -fomit-frame-pointer -fstrict-aliasing -funswitch-loops \
                       -fmodulo-sched -fmodulo-sched-allow-regmoves
TARGET_thumb_CFLAGS := -mthumb \
                        -Os \
                        -fomit-frame-pointer \
                        -fstrict-aliasing
TARGET_BOOTLOADER_BOARD_NAME := encore
TARGET_PROVIDES_INIT_TARGET_RC := true
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_OMAP3 := true
OMAP_ENHANCEMENT := true

ifdef OMAP_ENHANCEMENT
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP3 -DOMAP_ENHANCEMENT_CPCAM -DOMAP_ENHANCEMENT_VTC
endif

# Conserve memory in the Dalvik heap
# Details: https://github.com/CyanogenMod/android_dalvik/commit/15726c81059b74bf2352db29a3decfc4ea9c1428
TARGET_ARCH_LOWMEM := true

TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true

# for frameworks/native/opengl/libs
# disable use of unsupported EGL_KHR_gl_colorspace extension
BOARD_EGL_WORKAROUND_BUG_10194508 := true

# for frameworks/native/libs/gui
# disable use of EGL_KHR_fence_sync extension, since it slows things down
COMMON_GLOBAL_CFLAGS += -DDONT_USE_FENCE_SYNC

# for frameworks/native/services/surfaceflinger
# use EGL_IMG_context_priority extension, which helps performance
COMMON_GLOBAL_CFLAGS += -DHAS_CONTEXT_PRIORITY

TARGET_SPECIFIC_HEADER_PATH := device/bn/encore/include

# Makefile variables and C/C++ macros to recognise current pastry
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 16 || echo 1),)
    ANDROID_API_JB_OR_LATER := true
    COMMON_GLOBAL_CFLAGS += -DANDROID_API_JB_OR_LATER
endif
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 14 || echo 1),)
    ANDROID_API_ICS_OR_LATER := true
    COMMON_GLOBAL_CFLAGS += -DANDROID_API_ICS_OR_LATER
endif


BOARD_CUSTOM_BOOTIMG_MK := device/bn/encore/uboot-bootimg.mk
TARGET_RELEASETOOL_IMG_FROM_TARGET_SCRIPT := ./device/bn/encore/releasetools/encore_img_from_target_files
TARGET_RELEASETOOL_OTA_FROM_TARGET_SCRIPT := ./device/bn/encore/releasetools/encore_ota_from_target_files
TARGET_SYSTEMIMAGE_USE_SQUISHER := true
# Include a 2ndbootloader
TARGET_BOOTLOADER_IS_2ND := true

BOARD_KERNEL_CMDLINE := no_console_suspend=1 msmsdcc_sdioirq=1 wire.search_count=5
BOARD_KERNEL_BASE := 0x20000000
BOARD_KERNEL_IMAGE_NAME := uImage
BOARD_PAGE_SIZE := 0x00000800

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 461942784
BOARD_USERDATAIMAGE_PARTITION_SIZE := 987648000
BOARD_FLASH_BLOCK_SIZE := 4096
BOARD_USES_UBOOT := true

# Inline kernel building config
TARGET_KERNEL_CONFIG := encore_cm11_defconfig
TARGET_KERNEL_SOURCE := kernel/bn/encore

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
ifdef USES_TI_MAC80211
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X_TI
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wl12xx
BOARD_WLAN_DEVICE                := wl12xx_mac80211
BOARD_SOFTAP_DEVICE              := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME          := "wl12xx_sdio"
WIFI_FIRMWARE_LOADER             := ""
COMMON_GLOBAL_CFLAGS             += -DUSES_TI_MAC80211
BOARD_WIFI_SKIP_CAPABILITIES     := true
endif

TARGET_MODULES_SOURCE := "hardware/ti/wlan/mac80211/compat_wl12xx"

wifi_modules:
	make -C $(TARGET_MODULES_SOURCE) KERNEL_DIR=$(KERNEL_OUT) KLIB=$(KERNEL_OUT) KLIB_BUILD=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE)
	mv hardware/ti/wlan/mac80211/compat_wl12xx/net/mac80211/mac80211.ko $(KERNEL_MODULES_OUT)
	mv hardware/ti/wlan/mac80211/compat_wl12xx/net/wireless/cfg80211.ko $(KERNEL_MODULES_OUT)
	mv hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx.ko $(KERNEL_MODULES_OUT)
	mv hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx_sdio.ko $(KERNEL_MODULES_OUT)
	$(ARM_EABI_TOOLCHAIN)/arm-eabi-strip --strip-debug $(KERNEL_MODULES_OUT)/mac80211.ko $(KERNEL_MODULES_OUT)/cfg80211.ko $(KERNEL_MODULES_OUT)/wl12xx.ko $(KERNEL_MODULES_OUT)/wl12xx_sdio.ko

TARGET_KERNEL_MODULES := wifi_modules

BOARD_HAS_LARGE_FILESYSTEM := true
BOARD_RECOVERY_IGNORE_BOOTABLES := true
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/bn/encore/recovery/recovery_ui.c
TARGET_RECOVERY_PRE_COMMAND := "dd if=/dev/zero of=/rom/bcb bs=64 count=1 > /dev/null 2>&1 ; echo 'recovery' >> /rom/bcb ; sync"
TARGET_RECOVERY_FSTAB := device/bn/encore/fstab.encore
RECOVERY_FSTAB_VERSION := 2

# Modem
TARGET_NO_RADIOIMAGE := true

# HW Graphics (EGL fixes + webkit fix)
USE_OPENGL_RENDERER := true
BOARD_EGL_CFG := device/bn/encore/egl.cfg
ENABLE_WEBGL := true

# Storage
BOARD_HAS_SDCARD_INTERNAL := true
BOARD_SDCARD_DEVICE_PRIMARY := /dev/block/mmcblk1p1
BOARD_SDCARD_DEVICE_SECONDARY := /dev/block/mmcblk0p8
BOARD_SDCARD_DEVICE_INTERNAL := /dev/block/mmcblk0p8
BOARD_VOLD_MAX_PARTITIONS := 8
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/class/android_usb/android0/f_mass_storage/lun%d/file"

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_TI := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/bn/encore/bluetooth

BOARD_HAVE_FAKE_GPS := true

# MultiMedia defines
USE_CAMERA_STUB := true
BOARD_USES_TI_OMAP_MODEM_AUDIO := false
BOARD_HAS_NO_MISC_PARTITION := true

# Audio
# BOARD_USES_AUDIO_LEGACY := true
# TARGET_PROVIDES_LIBAUDIO := true
# BOARD_USES_GENERIC_AUDIO := false
# BOARD_USES_ALSA_AUDIO := true
BUILD_WITH_ALSA_UTILS := true

# If you'd like to build the audio components from source instead of using
# the prebuilts, uncomment BOARD_USES_ALSA_AUDIO := true above and add the
# following repositories to your manifest:
#
# <project name="steven676/android_hardware_alsa_sound" path="hardware/alsa_sound" remote="github" revision="jellybean-for-encore" />
# <project name="steven676/android_external_alsa-lib" path="external/alsa-lib" remote="github" revision="jellybean" />
#
# Also see encore.mk for further instructions.

HARDWARE_OMX := true

ifdef HARDWARE_OMX
OMX_JPEG := true
OMX_VENDOR := ti
TARGET_USE_OMX_RECOVERY := true
TARGET_USE_OMAP_COMPAT  := true
BUILD_WITH_TI_AUDIO := 1
BUILD_PV_VIDEO_ENCODERS := 1
OMX_VENDOR_INCLUDES := \
  hardware/ti/omap3/omx/system/src/openmax_il/omx_core/inc \
  hardware/ti/omap3/omx/image/src/openmax_il/jpeg_enc/inc
OMX_VENDOR_WRAPPER := TI_OMX_Wrapper
BOARD_OPENCORE_LIBRARIES := libOMX_Core
BOARD_OPENCORE_FLAGS := -DHARDWARE_OMX=1
#BOARD_CAMERA_LIBRARIES := libcamera
endif
      

BOARD_USES_SECURE_SERVICES := true
# Boot animation
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true

#adb has root
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0

#Config for building TWRP
DEVICE_RESOLUTION := 1024x600
RECOVERY_TOUCHSCREEN_SWAP_XY := true
RECOVERY_TOUCHSCREEN_FLIP_Y := true
TW_NO_REBOOT_BOOTLOADER := true
TW_NO_REBOOT_RECOVERY := true
TW_INTERNAL_STORAGE_PATH := "/emmc"
TW_INTERNAL_STORAGE_MOUNT_POINT := "emmc"
TW_EXTERNAL_STORAGE_PATH := "/sdc"
TW_EXTERNAL_STORAGE_MOUNT_POINT := "sdc"
