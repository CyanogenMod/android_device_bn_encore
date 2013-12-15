#
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

$(call inherit-product, device/bn/encore/full_encore.mk)

PRODUCT_RELEASE_NAME := NookColor

# We need screen width/height defined before inheriting
# common_full_tablet_wifionly.mk to avoid automatically bringing in the wrong
# boot animation.
TARGET_SCREEN_HEIGHT := 1024
TARGET_SCREEN_WIDTH := 600

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_mini_tablet_wifionly.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=encore BUILD_FINGERPRINT="bn/bn_encore/encore:4.4.2/KOT49H/937116:user/release-keys" PRIVATE_BUILD_DESC="encore-user 4.4.2 KOT49H 937116 release-keys"

PRODUCT_NAME := cm_encore
PRODUCT_DEVICE := encore
