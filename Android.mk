ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),encore)
include $(call first-makefiles-under,$(call my-dir))
endif
