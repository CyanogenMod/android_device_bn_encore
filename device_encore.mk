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

# Get a proper init file
PRODUCT_COPY_FILES += \
    device/bn/encore/init.encore.rc:root/init.encore.rc

# Place wifi files
PRODUCT_COPY_FILES += \
    device/bn/encore/prebuilt/wifi/tiwlan_drv.ko:/system/etc/wifi/tiwlan_drv.ko \
    device/bn/encore/prebuilt/wifi/tiwlan.ini:/system/etc/wifi/tiwlan.ini \
    device/bn/encore/prebuilt/wifi/firmware.bin:/system/etc/wifi/firmware.bin \

# Place permission files
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml

$(call inherit-product-if-exists, vendor/bn/encore/encore-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/bn/encore/overlay

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
    libVendor_ti_omx \
    sensors.encore \
    lights.encore

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

# POWERVR_SGX530_v125-binaries -- TI's GFX accel
PRODUCT_COPY_FILES += \
    device/bn/encore/prebuilt/GFX/system/bin/eglinfo:/system/bin/eglinfo \
    device/bn/encore/prebuilt/GFX/system/bin/framebuffer_test:/system/bin/framebuffer_test \
    device/bn/encore/prebuilt/GFX/system/bin/gles1test1:/system/bin/gles1test1 \
    device/bn/encore/prebuilt/GFX/system/bin/gles2test1:/system/bin/gles2test1 \
    device/bn/encore/prebuilt/GFX/system/bin/glsltest1_fragshaderA.txt:/system/bin/glsltest1_fragshaderA.txt \
    device/bn/encore/prebuilt/GFX/system/bin/glsltest1_fragshaderB.txt:/system/bin/glsltest1_fragshaderB.txt \
    device/bn/encore/prebuilt/GFX/system/bin/glsltest1_vertshader.txt:/system/bin/glsltest1_vertshader.txt \
    device/bn/encore/prebuilt/GFX/system/bin/hal_client_test:/system/bin/hal_client_test \
    device/bn/encore/prebuilt/GFX/system/bin/hal_server_test:/system/bin/hal_server_test \
    device/bn/encore/prebuilt/GFX/system/bin/pvr2d_test:/system/bin/pvr2d_test \
    device/bn/encore/prebuilt/GFX/system/bin/pvrsrvinit:/system/bin/pvrsrvinit \
    device/bn/encore/prebuilt/GFX/system/bin/services_test:/system/bin/services_test \
    device/bn/encore/prebuilt/GFX/system/bin/sgx/omaplfb.ko:/system/bin/sgx/omaplfb.ko \
    device/bn/encore/prebuilt/GFX/system/bin/sgx/pvrsrvkm.ko:/system/bin/sgx/pvrsrvkm.ko \
    device/bn/encore/prebuilt/GFX/system/bin/sgx/rc.pvr:/system/bin/sgx/rc.pvr \
    device/bn/encore/prebuilt/GFX/system/bin/sgx_blit_test:/system/bin/sgx_blit_test \
    device/bn/encore/prebuilt/GFX/system/bin/sgx_flip_test:/system/bin/sgx_flip_test \
    device/bn/encore/prebuilt/GFX/system/bin/sgx_init_test:/system/bin/sgx_init_test \
    device/bn/encore/prebuilt/GFX/system/bin/sgx_render_flip_test:/system/bin/sgx_render_flip_test \
    device/bn/encore/prebuilt/GFX/system/bin/texture_benchmark:/system/bin/texture_benchmark \
    device/bn/encore/prebuilt/GFX/system/bin/xmultiegltest:/system/bin/xmultiegltest \
    device/bn/encore/prebuilt/GFX/system/lib/egl/egl.cfg:/system/lib/egl/egl.cfg \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libEGL_POWERVR_SGX530_125.so:/system/lib/egl/libEGL_POWERVR_SGX530_125.so \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libEGL_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libEGL_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so:/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so:/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so \
    device/bn/encore/prebuilt/GFX/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so.1.1.15.2766:/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/hw/gralloc.omap3.so:/system/lib/hw/gralloc.omap3.so \
    device/bn/encore/prebuilt/GFX/system/lib/hw/gralloc.omap3.so.1.1.15.2766:/system/lib/hw/gralloc.omap3.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libfakehal.so:/system/lib/libfakehal.so \
    device/bn/encore/prebuilt/GFX/system/lib/libfakehal.so.1.1.15.2766:/system/lib/libfakehal.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libglslcompiler.so:/system/lib/libglslcompiler.so \
    device/bn/encore/prebuilt/GFX/system/lib/libglslcompiler.so.1.1.15.2766:/system/lib/libglslcompiler.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libIMGegl.so:/system/lib/libIMGegl.so \
    device/bn/encore/prebuilt/GFX/system/lib/libIMGegl.so.1.1.15.2766:/system/lib/libIMGegl.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libpvr2d.so:/system/lib/libpvr2d.so \
    device/bn/encore/prebuilt/GFX/system/lib/libpvr2d.so.1.1.15.2766:/system/lib/libpvr2d.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libpvrANDROID_WSEGL.so:/system/lib/libpvrANDROID_WSEGL.so \
    device/bn/encore/prebuilt/GFX/system/lib/libpvrANDROID_WSEGL.so.1.1.15.2766:/system/lib/libpvrANDROID_WSEGL.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libPVRScopeServices.so:/system/lib/libPVRScopeServices.so \
    device/bn/encore/prebuilt/GFX/system/lib/libPVRScopeServices.so.1.1.15.2766:/system/lib/libPVRScopeServices.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libsfutil.so:/system/lib/libsfutil.so \
    device/bn/encore/prebuilt/GFX/system/lib/libsfutil.so.1.1.15.2766:/system/lib/libsfutil.so.1.1.15.2766 \
    device/bn/encore/prebuilt/GFX/system/lib/libsrv_um.so:/system/lib/libsrv_um.so \
    device/bn/encore/prebuilt/GFX/system/lib/libsrv_um.so.1.1.15.2766:/system/lib/libsrv_um.so.1.1.15.2766

ifeq ($(TARGET_PREBUILT_KERNEL),)
	LOCAL_KERNEL := device/bn/encore/prebuilt/boot/kernel
else
	LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

# Set property overrides
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.lockprof.threshold=500 \
    dalvik.vm.dexopt-flags=m=y \
    ro.allow.mock.location=0 \
    ro.sf.lcd_density=160 \
    ro.setupwizard.enable_bypass=1 \
    ro.sf.hwrotation=270 \
    ro.com.google.networklocation=1 \
    ro.setupwizard.enable_bypass=1 \
	com.ti.omap_enhancement=true \
	opencore.asmd=1 \
	keyguard.no_require_sim=1 \
	wifi.interface=tiwlan0 \
	alsa.mixer.playback.master=DAC2 Analog \
	alsa.mixer.capture.master=Analog \
	dalvik.vm.heapsize=32m \
	ro.opengles.version=131072

$(call inherit-product, build/target/product/full.mk)

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := full_encore
PRODUCT_DEVICE := encore
