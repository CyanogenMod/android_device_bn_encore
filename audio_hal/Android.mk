# hardware/ti/omap3/modules/alsa/Android.mk
#
# Copyright 2009-2010 Texas Instruments
#

# This is the OMAP3 ALSA module for OMAP3
ifeq ($(strip $(TARGET_BOOTLOADER_BOARD_NAME)),encore)
ifeq ($(strip $(BOARD_USES_ALSA_AUDIO)),true)
  LOCAL_PATH := $(call my-dir)

  include $(CLEAR_VARS)

  LOCAL_PRELINK_MODULE := false

  LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

  LOCAL_CFLAGS := -D_POSIX_SOURCE -Wno-multichar
  ifeq ($(strip $(BOARD_USES_TI_OMAP_MODEM_AUDIO)),true)
    LOCAL_CFLAGS += -DAUDIO_MODEM_TI
  endif
  ifeq ($(BOARD_HAVE_BLUETOOTH),true)
      LOCAL_CFLAGS += -DAUDIO_BLUETOOTH
  endif

  LOCAL_C_INCLUDES += hardware/alsa_sound external/alsa-lib/include
  ifeq ($(strip $(BOARD_USES_TI_OMAP_MODEM_AUDIO)),true)
    LOCAL_C_INCLUDES += hardware/ti/omap3/modules/alsa
    ifeq ($(BOARD_HAVE_BLUETOOTH),true)
        LOCAL_C_INCLUDES += $(call include-path-for, bluez-libs) \
                            external/bluetooth/bluez/include
    endif
  endif

  ifeq ($(strip $(TARGET_BOARD_PLATFORM)), omap3)
    LOCAL_SRC_FILES:= alsa_omap3.cpp
    ifeq ($(strip $(BOARD_USES_TI_OMAP_MODEM_AUDIO)),true)
      LOCAL_SRC_FILES += alsa_omap3_modem.cpp
    endif
  endif
  ifeq ($(strip $(TARGET_BOARD_PLATFORM)), omap4)
    LOCAL_SRC_FILES:= alsa_omap4.cpp \
                       Omap4ALSAManager.cpp
    LOCAL_SHARED_LIBRARIES += libmedia
    ifeq ($(strip $(BOARD_USES_TI_OMAP_MODEM_AUDIO)),true)
      LOCAL_SRC_FILES += alsa_omap4_modem.cpp
    endif
  endif

  LOCAL_SHARED_LIBRARIES += \
    libaudio \
    libasound \
    liblog \
    libcutils \
    libutils \
    libdl

  ifeq ($(strip $(BOARD_USES_TI_OMAP_MODEM_AUDIO)),true)
    ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),true)
        LOCAL_SHARED_LIBRARIES += \
            libbluetooth

        #LOCAL_STATIC_LIBRARIES := \
         #   libbluez-common-static
    endif
  endif

  LOCAL_MODULE_TAGS:= optional
  LOCAL_MODULE:= alsa.$(TARGET_BOARD_PLATFORM)

  # XXX For some reason, read-only relocations cause the world to blow up
  # when loading this module
  # No significant benefit to marking relocations RO for this (not a
  # security-sensitive component), but it'd be nice to figure out why this
  # happens anyway ...
  LOCAL_LDFLAGS := -Wl,-z,norelro 

  include $(BUILD_SHARED_LIBRARY)

endif

# Build the audio.primary HAL which will be used by audioflinger to interface
# with the rest of the ALSA audio stack
# Makefile fragment comes from hardware/cm/audio/Android.mk
# Note that we don't actually need any extra code!
# XXX This really belongs inside the if BOARD_USES_ALSA_AUDIO, but as long as
# libaudio.so is a prebuilt we don't want to enable that option.

  include $(CLEAR_VARS)

  LOCAL_MODULE := audio.primary.$(TARGET_BOOTLOADER_BOARD_NAME)
  LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
  LOCAL_MODULE_TAGS := optional

  LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia \
    libhardware_legacy

  LOCAL_SHARED_LIBRARIES += libdl

  LOCAL_SHARED_LIBRARIES += libaudio

  LOCAL_STATIC_LIBRARIES := \
    libmedia_helper

  LOCAL_WHOLE_STATIC_LIBRARIES := \
    libaudiohw_legacy

  include $(BUILD_SHARED_LIBRARY)

endif
