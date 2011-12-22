$(call inherit-product, device/bn/encore/full_encore.mk)

PRODUCT_RELEASE_NAME := NookColor
# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=encore BUILD_ID=IML74K BUILD_DISPLAY_ID=IML74K BUILD_FINGERPRINT="bn/bn_encore/encore:2.3.4/IML74K/228551:user/release-keys"  PRIVATE_BUILD_DESC="encore-user 4.0.3 IML74K 228551 release-keys"

PRODUCT_NAME := cm_encore
PRODUCT_DEVICE := encore

