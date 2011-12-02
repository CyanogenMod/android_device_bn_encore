$(call inherit-product, device/bn/encore/full_encore.mk)

PRODUCT_RELEASE_NAME := NookColor
# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=encore BUILD_ID=GWK74 BUILD_FINGERPRINT=google/passion/passion:2.3.6/GRK39F/189904:user/release-keys PRIVATE_BUILD_DESC="passion-user 2.3.6 GRK39F 189904 release-keys" BUILD_NUMBER=189904

PRODUCT_NAME := cm_encore
PRODUCT_DEVICE := encore

