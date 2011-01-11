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

## Add in Softkeys -- source at http://git.hoopajoo.net/  GPLv3

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/prebuilt/app/SoftKeys_3.00.apk:/data/app/SoftKeys_3.00.apk

# cicadaman says /etc/wifi is the place for wifi drivers on TI machines...
# this driver is from TI.  Source at https://gforge.ti.com/gf/project/wilink_driver

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/prebuilt/wifi/tiwlan_drv.ko:/system/etc/wifi/tiwlan_drv.ko \
    device/bn/nookcolor/prebuilt/wifi/tiwlan.ini:/system/etc/wifi/tiwlan.ini \
    device/bn/nookcolor/prebuilt/wifi/firmware.bin:/system/etc/wifi/firmware.bin \
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

# POWERVR_SGX530_v125-binaries -- TI's GFX accel -- built and licensed by cicadman

PRODUCT_COPY_FILES += \
    device/bn/nookcolor/prebuilt/GFX/system/bin/eglinfo:/system/bin/eglinfo \
    device/bn/nookcolor/prebuilt/GFX/system/bin/framebuffer_test:/system/bin/framebuffer_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/gles1test1:/system/bin/gles1test1 \
    device/bn/nookcolor/prebuilt/GFX/system/bin/gles2test1:/system/bin/gles2test1 \
    device/bn/nookcolor/prebuilt/GFX/system/bin/glsltest1_fragshaderA.txt:/system/bin/glsltest1_fragshaderA.txt \
    device/bn/nookcolor/prebuilt/GFX/system/bin/glsltest1_fragshaderB.txt:/system/bin/glsltest1_fragshaderB.txt \
    device/bn/nookcolor/prebuilt/GFX/system/bin/glsltest1_vertshader.txt:/system/bin/glsltest1_vertshader.txt \
    device/bn/nookcolor/prebuilt/GFX/system/bin/hal_client_test:/system/bin/hal_client_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/hal_server_test:/system/bin/hal_server_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/pvr2d_test:/system/bin/pvr2d_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/pvrsrvinit:/system/bin/pvrsrvinit \
    device/bn/nookcolor/prebuilt/GFX/system/bin/services_test:/system/bin/services_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx/omaplfb.ko:/system/bin/sgx/omaplfb.ko \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx/pvrsrvkm.ko:/system/bin/sgx/pvrsrvkm.ko \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx/rc.pvr:/system/bin/sgx/rc.pvr \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx_blit_test:/system/bin/sgx_blit_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx_flip_test:/system/bin/sgx_flip_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx_init_test:/system/bin/sgx_init_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/sgx_render_flip_test:/system/bin/sgx_render_flip_test \
    device/bn/nookcolor/prebuilt/GFX/system/bin/texture_benchmark:/system/bin/texture_benchmark \
    device/bn/nookcolor/prebuilt/GFX/system/bin/xmultiegltest:/system/bin/xmultiegltest \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/egl.cfg:/system/lib/egl/egl.cfg \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libEGL_POWERVR_SGX530_125.so:/system/lib/egl/libEGL_POWERVR_SGX530_125.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libEGL_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libEGL_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so:/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so:/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/hw/gralloc.omap3.so:/system/lib/hw/gralloc.omap3.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/hw/gralloc.omap3.so.1.1.15.2766:/system/lib/hw/gralloc.omap3.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libfakehal.so:/system/lib/libfakehal.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libfakehal.so.1.1.15.2766:/system/lib/libfakehal.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libglslcompiler.so:/system/lib/libglslcompiler.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libglslcompiler.so.1.1.15.2766:/system/lib/libglslcompiler.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libIMGegl.so:/system/lib/libIMGegl.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libIMGegl.so.1.1.15.2766:/system/lib/libIMGegl.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libpvr2d.so:/system/lib/libpvr2d.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libpvr2d.so.1.1.15.2766:/system/lib/libpvr2d.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libpvrANDROID_WSEGL.so:/system/lib/libpvrANDROID_WSEGL.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libpvrANDROID_WSEGL.so.1.1.15.2766:/system/lib/libpvrANDROID_WSEGL.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libPVRScopeServices.so:/system/lib/libPVRScopeServices.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libPVRScopeServices.so.1.1.15.2766:/system/lib/libPVRScopeServices.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libsfutil.so:/system/lib/libsfutil.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libsfutil.so.1.1.15.2766:/system/lib/libsfutil.so.1.1.15.2766 \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libsrv_um.so:/system/lib/libsrv_um.so \
    device/bn/nookcolor/prebuilt/GFX/system/lib/libsrv_um.so.1.1.15.2766:/system/lib/libsrv_um.so.1.1.15.2766

