#
# Copyright (C) 2009 The Android Open Source Project
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
#

# include cicada's sensors library
common_ti_dirs := libsensors

include $(call all-named-subdir-makefiles, $(common_ti_dirs))

$(call inherit-product, build/target/product/full_base.mk)

# Get a proper init file
PRODUCT_COPY_FILES += \
    device/bn/encore/init.encore.rc:root/init.encore.rc

# Place wifi files
PRODUCT_COPY_FILES += \
    device/bn/encore/prebuilt/wifi/tiwlan_drv.ko:/system/lib/modules/tiwlan_drv.ko \
    device/bn/encore/prebuilt/wifi/tiwlan.ini:/system/etc/wifi/tiwlan.ini \
    device/bn/encore/prebuilt/wifi/firmware.bin:/system/etc/wifi/firmware.bin

# Place bluetooth firmware
PRODUCT_COPY_FILES += \
    device/bn/encore/firmware/TIInit_7.2.31.bts:/system/etc/firmware/TIInit_7.2.31.bts

# Place prebuilt from omapzoom
PRODUCT_COPY_FILES += \
	device/bn/encore/prebuilt/GFX/overlay.omap3.so:/system/lib/hw/overlay.omap3.so
	device/bn/encore/prebuilt/alsa/alsa.omap3.so:/system/lib/hw/alsa.omap3.so

# Place permission files
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml

$(call inherit-product-if-exists, vendor/bn/encore/encore-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/bn/encore/overlay

PRODUCT_PACKAGES += \
    librs_jni \
    tiwlan.ini \
    dspexec \
    libbridge \
    wlan_cu \
    libtiOsLib \
    wlan_loader \
    libCustomWifi \
    wpa_supplicant.conf \
    dhcpcd.conf \
    libOMX.TI.AAC.encode \
    libOMX.TI.AMR.encode \
    libOMX.TI.WBAMR.encode \
    libOMX.TI.JPEG.Encoder \
    libLCML \
    libOMX_Core \
    libOMX.TI.Video.Decoder \
    libOMX.TI.Video.encoder \
    libVendor_ti_omx \
    sensors.encore \
    lights.encore \
    alsa.default \
    acoustics.default

# Use medium-density artwork where available
PRODUCT_LOCALES += mdpi

# Vold
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/etc/vold.encore.fstab:system/etc/vold.fstab

# Media Profile
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/etc/media_profiles.xml:system/etc/media_profiles.xml

# Misc # TODO: Find a better home for this
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/clear_bootcnt.sh:/system/bin/clear_bootcnt.sh

# SD ramdisk packer script - by request - execute manually as-needed

PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/sd_ramdisk_packer.sh:sd_ramdisk_packer.sh

ifeq ($(TARGET_PREBUILT_KERNEL),)
    LOCAL_KERNEL := device/bn/encore/prebuilt/boot/kernel
else
    LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

# Set property overrides
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dexopt-flags=m=y \
    ro.com.google.locationfeatures=1 \
    ro.com.google.networklocation=1 \
    ro.allow.mock.location=1 \
    qemu.sf.lcd_density=161 \
    ro.setupwizard.enable_bypass=1 \
    ro.sf.hwrotation=270 \
    ro.setupwizard.enable_bypass=1 \
    keyguard.no_require_sim=1 \
    wifi.interface=tiwlan0 \
    alsa.mixer.playback.master=default \
    alsa.mixer.capture.master=Analog \
    ro.opengles.version=131072

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := full_encore
PRODUCT_DEVICE := encore
