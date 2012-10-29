ifeq ($(strip $(TARGET_BOOTLOADER_BOARD_NAME)),encore)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := store-mac-addr.sh
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

#
# Install a symlink in /system/etc/firmware pointing to the eventual location
# of the modified nvs file on the device.  Snippet shamelessly stolen from
# external/busybox/Android.mk.
#
# If we didn't need this, there would be no need for this Makefile -- we'd just
# add the shell script to PRODUCT_COPY_FILES and be done with it.
#

NVS_FILENAME := wl1271-nvs.bin
NVS_SYMLINK_TARGET := /data/misc/wifi/$(NVS_FILENAME)
NVS_SYMLINK_DIR := $(TARGET_OUT_ETC)/firmware/ti-connectivity
NVS_SYMLINK := $(NVS_SYMLINK_DIR)/$(NVS_FILENAME)
$(NVS_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@rm -rf $@
	@mkdir -p $(NVS_SYMLINK_DIR)
	ln -sf $(NVS_SYMLINK_TARGET) $@

ALL_DEFAULT_INSTALLED_MODULES += $(NVS_SYMLINK)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
	$(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(NVS_SYMLINK)

endif
