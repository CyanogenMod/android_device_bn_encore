# Copyright (C) 2010 Ricardo Cerqueira
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


ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),encore)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := fsfinder.c

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_STATIC_LIBRARIES := libc

LOCAL_MODULE := fsfinder
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

RECOVERY_FSFINDER := $(TARGET_RECOVERY_ROOT_OUT)/sbin/$(LOCAL_MODULE)
RECOVERY_FSFINDER_SRC := $(TARGET_ROOT_OUT_SBIN)/$(LOCAL_MODULE)
$(RECOVERY_FSFINDER): $(LOCAL_INSTALLED_MODULE)
	rm -f $@
	mkdir -p $(TARGET_RECOVERY_ROOT_OUT)/sbin
	cp -p $(RECOVERY_FSFINDER_SRC) $@

ALL_DEFAULT_INSTALLED_MODULES += $(RECOVERY_FSFINDER)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
	        $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(RECOVERY_FSFINDER)



include $(CLEAR_VARS)

LOCAL_MODULE := configure_vold.sh
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Change the interpreter path for the recovery's copy of the script
RECOVERY_CONFIGURE_VOLD := $(TARGET_RECOVERY_ROOT_OUT)/sbin/$(LOCAL_MODULE)
RECOVERY_CONFIGURE_VOLD_SRC := $(TARGET_OUT_EXECUTABLES)/$(LOCAL_MODULE)
$(RECOVERY_CONFIGURE_VOLD): $(LOCAL_INSTALLED_MODULE)
	echo "#!/sbin/sh" > $@
	tail -n +2 $(RECOVERY_CONFIGURE_VOLD_SRC) >> $@
	chmod 0755 $@

ALL_DEFAULT_INSTALLED_MODULES += $(RECOVERY_CONFIGURE_VOLD)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
	        $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(RECOVERY_CONFIGURE_VOLD)

include $(CLEAR_VARS)
LOCAL_MODULE := recovery_emmc_protect.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_SRC_FILES := recovery_emmc_protect.sh
include $(BUILD_PREBUILT)

endif
