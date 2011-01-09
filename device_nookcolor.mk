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

DEVICE_PACKAGE_OVERLAYS += device/bn/nookcolor/overlay

#
# This is the product configuration for a generic GSM passion,
# not specialized for any geography.
#

#$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# include cicada's sensors library

common_ti_dirs := sensors
include $(call all-named-subdir-makefiles, $(common_ti_dirs))

# The gps config appropriate for this device
# Doesn't apply to NC
#$(call inherit-product, device/common/gps/gps_us_supl.mk)

## (1) First, the most specific values, i.e. the aspects that are specific to GSM

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/init.nookcolor.rc:root/init.nookcolor.rc

## (2) Also get non-open-source GSM-specific aspects if available
$(call inherit-product-if-exists, vendor/bn/nookcolor/nookcolor-vendor.mk)

## (3)  Finally, the least specific parts, i.e. the non-GSM-specific aspects
PRODUCT_PROPERTY_OVERRIDES += \
        ro.com.android.wifi-watchlist=GoogleGuest \
        ro.error.receiver.system.apps=com.google.android.feedback \
        ro.setupwizard.enterprise_mode=1 \
        ro.com.google.clientidbase=android-verizon \
        ro.com.google.locationfeatures=1 \
        ro.url.legal=http://www.google.com/intl/%s/mobile/android/basic/phone-legal.html \
        ro.url.legal.android_privacy=http://www.google.com/intl/%s/mobile/android/basic/privacy.html \
        dalvik.vm.lockprof.threshold=500 \
        dalvik.vm.dexopt-flags=m=y \
        ro.allow.mock.location=0 \
        ro.sf.lcd_density=160 \
        ro.setupwizard.enable_bypass=1 \
        ro.sf.hwrotation=270 \
        dalvik.vm.heapsize=48m \
        ctl.stop ril-daemon \
	com.ti.omap_enhancement=true \
	opencore.asmd=1 \
	keyguard.no_require_sim=1 \
	wifi.interface=tiwlan0 \
	alsa.mixer.playback.master=DAC2 Analog \
	alsa.mixer.capture.master=Analog \
	dalvik.vm.heapsize=32m \
	ro.opengles.version=131072


                                        
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
#    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml
#    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
#    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
#    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
#    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \



# media config xml file
PRODUCT_COPY_FILES += \
    device/bn/nookcolor/media_profiles.xml:system/etc/media_profiles.xml

PRODUCT_PACKAGES += \
    librs_jni \
    tiwlan.ini \
    dspexec \
    libbridge \
    overlay.omap3 \
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
    libVendor_ti_omx
#    gps.nookcolor
#    libcamera

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# Use high-density artwork where available
PRODUCT_LOCALES += hdpi

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/vold.fstab:system/etc/vold.fstab

# cicadaman says /etc/wifi is the place for wifi drivers on TI machines...
# this driver is from TI.  Source at https://gforge.ti.com/gf/project/wilink_driver

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/tiwlan_drv.ko:/system/etc/wifi/tiwlan_drv.ko \
    device/bn/nookcolor/tiwlan.ini:/system/etc/wifi/tiwlan.ini \
    device/bn/nookcolor/firmware.bin:/system/etc/wifi/firmware.bin \
    device/bn/nookcolor/clear_bootcnt.sh:/system/bin/clear_bootcnt.sh

# cicadaman's custom accel lib

#PRODUCT_COPY_FILES += \
#    device/bn/nookcolor/sensors.omap3.so:/system/lib/hw/


ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/bn/nookcolor/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

$(call inherit-product-if-exists, vendor/bn/nookcolor/nookcolor-vendor.mk)

# stuff common to all HTC phones
#$(call inherit-product, device/htc/common/common.mk)

$(call inherit-product, build/target/product/full.mk)

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := full_nookcolor
PRODUCT_DEVICE := nookcolor
#TARGET_BUILD_TYPE:=debug
