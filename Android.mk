#
# Copyright (C) 2011 The Android Open Source Project
# Copyright (C) 2012 The CyanogenMod Project
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

# WARNING: Everything listed here will be built on ALL platforms,
# including x86, the emulator, and the SDK.  Modules must be uniquely
# named (liblights.encore), and must build everywhere, or limit themselves
# to only building on ARM if they include assembly. Individual makefiles
# are responsible for having their own logic, for fine-grained control.

LOCAL_PATH := $(call my-dir)

# if some modules are built directly from this directory (not subdirectories),
# their rules should be written here.

ifneq ($(filter encore,$(TARGET_DEVICE)),)

ifneq ($(TARGET_SIMULATOR),true)
include $(call all-makefiles-under,$(LOCAL_PATH))

# XXX UGLY workaround to avoid running out of space when installing
# Google Apps packages
$(TARGET_OUT_APPS)/GoogleHome.apk:
	@rm -rf $@
	@mkdir -p $(TARGET_OUT_APPS)
	ln -sf /dev/null $@

$(TARGET_OUT_APPS)/Hangouts.apk:
	@rm -rf $@
	@mkdir -p $(TARGET_OUT_APPS)
	ln -sf /dev/null $@

ALL_DEFAULT_INSTALLED_MODULES += \
	$(TARGET_OUT_APPS)/GoogleHome.apk \
	$(TARGET_OUT_APPS)/Hangouts.apk

endif

endif
