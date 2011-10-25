/* alsa_omap4.h
 **
 ** Copyright (C) 2009-2011, Texas Instruments
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_ALSA_OMAP4
#define ANDROID_ALSA_OMAP4

#include "Omap4ALSAManager.h"

#ifdef AUDIO_MODEM_TI
#include "audio_modem_interface.h"
#include "alsa_omap4_modem.h"
#endif

// alsa devices
#define MM_DEFAULT_DEVICE     "plughw:0,0"
#define BLUETOOTH_SCO_DEVICE  "plughw:0,0"
#define FM_TRANSMIT_DEVICE	  "plughw:0,0"
#define FM_CAPTURE_DEVICE     "plughw:0,1"
#define MM_LP_DEVICE          "hw:0,6"
#define HDMI_DEVICE	          "plughw:0,7"

// omap4 outputs/inputs
#define OMAP4_OUT_SCO      (\
        AudioSystem::DEVICE_OUT_BLUETOOTH_SCO |\
        AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET |\
        AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT)

#define OMAP4_OUT_FM        (\
        AudioSystem::DEVICE_OUT_FM_TRANSMIT)

#define OMAP4_OUT_HDMI        (\
        AudioSystem::DEVICE_OUT_AUX_DIGITAL)

#define OMAP4_OUT_LP        (\
        AudioSystem::DEVICE_OUT_LOW_POWER)

#define OMAP4_OUT_DEFAULT   (\
        AudioSystem::DEVICE_OUT_ALL &\
        ~OMAP4_OUT_SCO &\
        ~OMAP4_OUT_FM  &\
        ~OMAP4_OUT_LP  &\
        ~OMAP4_OUT_HDMI)

#define OMAP4_IN_SCO        (\
        AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET)

#define OMAP4_IN_FM        (\
        AudioSystem::DEVICE_IN_FM_ANALOG)

#define OMAP4_IN_DEFAULT    (\
        AudioSystem::DEVICE_IN_ALL &\
        ~OMAP4_IN_SCO)


#ifndef ALSA_DEFAULT_SAMPLE_RATE
#define ALSA_DEFAULT_SAMPLE_RATE 48000 // in Hz
#endif

#ifndef MM_LP_SAMPLE_RATE
 //not used for now
#define MM_LP_SAMPLE_RATE 44100        // in Hz
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif    // ANDROID_ALSA_OMAP4
