/* alsa_omap3_modem.cpp
 **
 ** Copyright 2009-2010 Texas Instruments
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

#define LOG_TAG "Omap3ALSAModem"
#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <cutils/properties.h>
#include <dlfcn.h>

#ifdef AUDIO_BLUETOOTH
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sco.h>
#endif

#include "AudioHardwareALSA.h"
#include "audio_modem_interface.h"
#include "alsa_omap3_modem.h"

namespace android
{
// Voice call volume
#define VOICE_CALL_VOLUME_PROP(dev, name) \
    {\
        dev, name, NULL\
    }

static voiceCallVolumeList
voiceCallVolumeProp[] = {
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HANDSET,
                            "DAC Voice Digital Downlink Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HANDFREE,
                            "DAC Voice Digital Downlink Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HEADSET,
                            "DAC Voice Digital Downlink Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_AUX,
                            "DAC Voice Digital Downlink Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_BLUETOOTH,
                            "BT Digital Playback Volume"),
    VOICE_CALL_VOLUME_PROP(0, "")
};
// ----------------------------------------------------------------------------

AudioModemAlsa::AudioModemAlsa(ALSAControl *alsaControl)
{
    status_t error;

    LOGV("Build date: %s time: %s", __DATE__, __TIME__);
    LOGD("Initializing devices for Modem OMAP3 ALSA module");

    audioModemSetProperties();
    mModem = create();
    if (mModem) {
        error = mModem->initCheck() ;
        if (error != NO_ERROR) {
            LOGE("Audio Modem Interface was not correctly initialized.");
            delete mModem;
            exit(error);
        }
    }
    else {
        LOGE("No Audio Modem Interface found.");
        exit(-1);
    }
    mVoiceCallState = AUDIO_MODEM_VOICE_CALL_OFF;

    // Initialize Min and Max volume
    int i = 0;
    while(voiceCallVolumeProp[i].device) {
        voiceCallVolumeInfo *info = voiceCallVolumeProp[i].mInfo = new voiceCallVolumeInfo;

        error = alsaControl->getmax(voiceCallVolumeProp[i].volumeName, info->max);
        error = alsaControl->getmin(voiceCallVolumeProp[i].volumeName, info->min);
        LOGV("Voice call volume name: %s min: %d max: %d", voiceCallVolumeProp[i].volumeName,
                 info->min, info->max);

        if (error != NO_ERROR) {
            LOGE("Audio Voice Call volume was not correctly initialized.");
            delete mModem;
            exit(error);
        }
        i++;
    }
}

AudioModemAlsa::~AudioModemAlsa()
{
    LOGD("Destroy devices for Modem OMAP3 ALSA module");
    mDevicePropList.clear();
    if (mModem) delete mModem;
}

AudioModemInterface* AudioModemAlsa::create()
{
    void *dlHandle;
    char libPath[PROPERTY_VALUE_MAX];
    AudioModemInterface *(*audioModem)(void);

    property_get(AUDIO_MODEM_LIB_PATH_PROPERTY,
                 libPath,
                 AUDIO_MODEM_LIB_DEFAULT_PATH);

    LOGW_IF(!strcmp(libPath, AUDIO_MODEM_LIB_DEFAULT_PATH),
                    "Use generic Modem interface");

    dlHandle = dlopen(libPath, RTLD_NOW);
    if (dlHandle == NULL) {
        LOGE("Audio Modem %s dlopen failed: %s\n", libPath, dlerror());
        exit(-1);
    }

    audioModem = (AudioModemInterface *(*)(void))dlsym(dlHandle, "createAudioModemInterface");
    if (audioModem == NULL) {
        LOGE("createAudioModemInterface function not defined or not exported in %s\n", libPath);
        exit(-1);
    }

    return(audioModem());
}

AudioModemAlsa::AudioModemDeviceProperties::AudioModemDeviceProperties(
                                                    const char *audioModeName)
{
    if (!strcmp(audioModeName,".hand")) audioMode =
                                    AudioModemInterface::AUDIO_MODEM_HANDSET;
    else if (!strcmp(audioModeName,".free")) audioMode =
                                    AudioModemInterface::AUDIO_MODEM_HANDFREE;
    else if (!strcmp(audioModeName,".head")) audioMode =
                                    AudioModemInterface::AUDIO_MODEM_HEADSET;
    else if (!strcmp(audioModeName,".bt")) audioMode =
                                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH;
    else if (!strcmp(audioModeName,".aux")) audioMode =
                                    AudioModemInterface::AUDIO_MODEM_AUX;
}

void AudioModemAlsa::AudioModemDeviceProperties::initProperties(int index,
                                                                char *propValue,
                                                                bool def)
{
    if (def) {
        strcpy((char *)settingsList[index].name,
               AUDIO_MODEM_SETTING_DEFAULT);
    } else {
        strcpy((char *)settingsList[index].name, propValue);
    }
}

status_t AudioModemAlsa::audioModemSetProperties()
{
    int i, j, k, index = 0;
    char propKey[PROPERTY_KEY_MAX], propValue[PROPERTY_VALUE_MAX];
    AudioModemDeviceProperties *deviceProp;

    mDevicePropList.clear();

    for (i = 0; i < audioModeNameLen; i++) {
        deviceProp = new AudioModemDeviceProperties(audioModeName[i]);
        index = 0;
        for (j = 0; j < modemStreamNameLen; j++) {
            for (k = 0; k < settingNameLen; k++) {
                if (settingName[k].streamUsed & (1 << j)) {
                    // modem.<audio_mode>.<stream>.<setting>
                    strcpy (propKey, PROP_BASE_NAME);
                    strcpy (propValue, "");
                    strcat (propKey, audioModeName[i]);
                    strcat (propKey, modemStreamName[j]);
                    strcat (propKey, settingName[k].name);

                    property_get(propKey, propValue, "");

                    if (!strcmp(propValue, "")) {
                        deviceProp->initProperties(index,
                                                    propValue,
                                                    true);
                    } else {
                        deviceProp->initProperties(index,
                                                    propValue,
                                                    false);
                    }

                    LOGV("%s = %s", propKey,
                                    deviceProp->settingsList[index].name);
                    index++;
                }
            }
        }
        mDevicePropList.add(deviceProp->audioMode, deviceProp);
    }
    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallControls(uint32_t devices, int mode,
                                           ALSAControl *alsaControl)
{
    status_t error = NO_ERROR;

LOGV("%s: devices %04x mode %d", __FUNCTION__, devices, mode);

    mAlsaControl = alsaControl;

    if ((mode == AudioSystem::MODE_IN_CALL) &&
        (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_OFF)) {
        // enter in voice call mode
        error = setCurrentAudioModemModes(devices);
        if (error < 0) return error;
        mDeviceProp = mDevicePropList.valueFor(mCurrentAudioModemModes);
        error = voiceCallCodecSet();
        if (error < 0) return error;
        error = voiceCallModemSet();
        if (error < 0) return error;
#ifdef AUDIO_BLUETOOTH
        if (mCurrentAudioModemModes ==
                AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
            error = voiceCallBTDeviceEnable();
            if (error < 0) return error;
        }
#endif
        mVoiceCallState = AUDIO_MODEM_VOICE_CALL_ON;
    } else if ((mode == AudioSystem::MODE_IN_CALL) &&
               (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_ON)) {
        // update in voice call mode
        mPreviousAudioModemModes = mCurrentAudioModemModes;
        error = setCurrentAudioModemModes(devices);
        if (error < 0) {
            mCurrentAudioModemModes = mPreviousAudioModemModes;
            return error;
        }
        mDevicePropPrevious = mDeviceProp;
        mDeviceProp = mDevicePropList.valueFor(mCurrentAudioModemModes);
        if (mCurrentAudioModemModes != mPreviousAudioModemModes) {
            error = voiceCallCodecUpdate();
            if (error < 0) return error;
            error = voiceCallModemUpdate();
            if (error < 0) return error;
#ifdef AUDIO_BLUETOOTH
            if (mCurrentAudioModemModes ==
                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
                error = voiceCallBTDeviceEnable();
                if (error < 0) return error;
            } else if (mPreviousAudioModemModes ==
                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
                error = voiceCallBTDeviceDisable();
                if (error < 0) return error;
            }
#endif
        } else {
            LOGI("Audio Modem Mode doesn't changed: no update needed");
        }
    } else if ((mode != AudioSystem::MODE_IN_CALL) &&
               (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_ON)) {
        // we just exit voice call mode
        mPreviousAudioModemModes = mCurrentAudioModemModes;
        mDevicePropPrevious = mDeviceProp;
        error = voiceCallCodecReset();
        if (error < 0) return error;
        error = voiceCallModemReset();
        if (error < 0) return error;
#ifdef AUDIO_BLUETOOTH
        if (mPreviousAudioModemModes ==
                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
                error = voiceCallBTDeviceDisable();
                if (error < 0) return error;
        }
#endif
        mVoiceCallState = AUDIO_MODEM_VOICE_CALL_OFF;
    }

    return error;
}

status_t AudioModemAlsa::setCurrentAudioModemModes(uint32_t devices)
{
    if (devices & AudioModemInterface::AUDIO_MODEM_HANDSET) {
        mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_HANDSET;
    } else if (devices & AudioModemInterface::AUDIO_MODEM_HANDFREE) {
        mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_HANDFREE;
    } else if (devices & AudioModemInterface::AUDIO_MODEM_HEADSET) {
        mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_HEADSET;
    } else if (devices & AudioModemInterface::AUDIO_MODEM_AUX) {
        mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_AUX;
#ifdef AUDIO_BLUETOOTH
    } else if (devices & AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_BLUETOOTH;
#endif
    } else {
        LOGE("Devices %04x not supported", devices);
        return NO_INIT;
    }
    LOGV("New Audio Modem Modes: %04x", mCurrentAudioModemModes);
    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallCodecSet()
{
    status_t error = NO_ERROR;

    LOGV("Start Audio Codec Voice call: %04x", mCurrentAudioModemModes);
    error = mAlsaControl->set("Codec Operation Mode", "Option 2 (voice/audio)");

    if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDSET) {
        error = voiceCallCodecSetHandset();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDFREE) {
        error = voiceCallCodecSetHandfree();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HEADSET) {
        error = voiceCallCodecSetHeadset();
#ifdef AUDIO_BLUETOOTH
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        error = voiceCallCodecSetBluetooth();
#endif
    } else {
        LOGE("Audio Modem mode not supported: %d", mCurrentAudioModemModes);
        return INVALID_OPERATION;
    }

    error = voiceCallCodecPCMSet();

    return error;
}

status_t AudioModemAlsa::voiceCallCodecPCMSet()
{
    status_t error = NO_ERROR;

    // PCM voice port config
    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        error = mAlsaControl->set("Voice Sample Rate", "16 kHz");
    } else {
        error = mAlsaControl->set("Voice Sample Rate", "8 kHz");
    }

    error = mAlsaControl->set("Voice Clock Mode", "Master Mode");
    error = mAlsaControl->set("Voice I/O Swap", "VDX/VDR not swapped");
    error = mAlsaControl->set("Voice Interface Mode", "Mode 1 (writing on PCM_VCK rising edge)");
    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        error = mAlsaControl->set("Voice Microphone Mode", "Dual Microphone");
    } else {
        error = mAlsaControl->set("Voice Microphone Mode", "Mono Microphone");
    }
    error = mAlsaControl->set("Voice Tristate Switch", 0, 0);
    error = mAlsaControl->set("Voice Input Switch", 1, 0);
    error = mAlsaControl->set("Voice Ouput Switch", 1, 0);
    error = mAlsaControl->set("Voice Switch", 1, 0);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecPCMUpdate()
{
    status_t error = NO_ERROR;

    // PCM voice port config update
    if ((!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) &&
        (strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz"))        ) {
        error = mAlsaControl->set("Voice Sample Rate", "16 kHz");
    } else {
        error = mAlsaControl->set("Voice Sample Rate", "8 kHz");
    }

        if ((!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) &&
        (strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes"))) {
        error = mAlsaControl->set("Voice Microphone Mode", "Dual Microphone");
    } else {
        error = mAlsaControl->set("Voice Microphone Mode", "Mono Microphone");
    }

    return error;
}


status_t AudioModemAlsa::voiceCallCodecPCMReset()
{
    status_t error = NO_ERROR;

    // Disable PCM voice port
    error = mAlsaControl->set("Voice Tristate Switch", 1, 0);
    error = mAlsaControl->set("Voice Input Switch", 0, 0);
    error = mAlsaControl->set("Voice Ouput Switch", 0, 0);
    error = mAlsaControl->set("Voice Switch", 0, 0);

    return error;
}
#ifdef AUDIO_BLUETOOTH
status_t AudioModemAlsa::voiceCallCodecBTPCMSet()
{
    status_t error = NO_ERROR;

    error = mAlsaControl->set("BT Digital Playback Volume",
                             AUDIO_CODEC_VOICE_DIGITAL_VOL_BLUETOOTH, 0);
    error = mAlsaControl->set("BT Digital Capture Volume",
                             AUDIO_CODEC_VOICE_DIGITAL_CAPTURE_VOL_BLUETOOTH, 0);
    error = mAlsaControl->set("BT Sidetone Volume",
                             AUDIO_CODEC_SIDETONE_GAIN_BLUETOOTH, 0);

    error = mAlsaControl->set("BT data out playback MUX", "VDR data");
    error = mAlsaControl->set("Voice data out MUX", "BT data");

    error = mAlsaControl->set("BT I/O Swap", "BTVDX/BTVDR not swapped");

    error = mAlsaControl->set("BT Tristate Switch", 0, 0);
    error = mAlsaControl->set("Enable BT input", 1, 0);
    error = mAlsaControl->set("Enable BT output", 1, 0);
    error = mAlsaControl->set("BT Interface Switch", 1, 0);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecBTPCMReset()
{
    status_t error = NO_ERROR;

    // Disable BT path
    error = mAlsaControl->set("BT Digital Playback Volume",
                             0, 0);
    error = mAlsaControl->set("BT Digital Capture Volume",
                             0, 0);
    error = mAlsaControl->set("BT Sidetone Volume",
                             0, 0);

    error = mAlsaControl->set("BT data out playback MUX", "VTx data");
    error = mAlsaControl->set("Voice data out MUX", "VTx data");

    error = mAlsaControl->set("BT Tristate Switch", 1, 0);
    error = mAlsaControl->set("Enable BT input", 0, 0);
    error = mAlsaControl->set("Enable BT output", 0, 0);
    error = mAlsaControl->set("BT Interface Switch", 0, 0);

    return error;
}

status_t AudioModemAlsa::voiceCallBTDeviceEnable()
{
    status_t error = NO_ERROR;

    LOGV("Enable PCM port of Bluetooth SCO device");

    error = system("/system/xbin/bt_voice_call_set");

    LOGE_IF(error < 0, "Problem during bt_voice_call_set call: %s(%d)",
            strerror(error), error);

    return error;
}

status_t AudioModemAlsa::voiceCallBTDeviceDisable()
{
    status_t error = NO_ERROR;

    LOGV("Disable PCM port of Bluetooth SCO device");

    return error;
}
#endif // AUDIO_BLUETOOTH

status_t AudioModemAlsa::voiceCallCodecSetHandset()
{
    status_t error = NO_ERROR;

    // On Zoom2 the Headset mode doesn't exist only Handfree mode exist
    if (1) {
        error = voiceCallCodecSetHandfree();
    } else {
        // Capture path
        error = mAlsaControl->set("Microphone Offset Cancellation Mode", "Voice");

        error = mAlsaControl->set("Analog Capture Main Mic Switch", 1, 0);
        error = mAlsaControl->set("Main Microphone Bias Switch", 1, 0);
        error = mAlsaControl->set("Analog Capture Volume",
                                AUDIO_CODEC_ANAMIC_GAIN_HANDSET_L, 0);
        error = mAlsaControl->set("Analog Capture Left Switch", 1, 0);
        error = mAlsaControl->set("TX2 Capture Route", "Analog");
        error = mAlsaControl->set("AVADC Clock Priority", "Voice high priority");
        error = mAlsaControl->set("ADC Voice Analog Left Capture Power", 1, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                                AUDIO_CODEC_TXL2_VOL_HANDSET, 0);
        error = mAlsaControl->set("Voice Digital Capture Left Filter Switch",
                                1, 0);

        if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) {
            LOGV("dual mic. enabled");
            error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
            error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
            error = mAlsaControl->set("Analog Capture Volume",
                                    AUDIO_CODEC_ANAMIC_GAIN_HANDSET_R, 1);
            error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
            error = mAlsaControl->set("TX1 Capture Route", "Analog");
            error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
            error = mAlsaControl->set("TX2 Digital Capture Volume",
                                AUDIO_CODEC_TXR2_VOL_HANDSET, 1);
            error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                    1, 0);
        }

        // Playback path
        error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                                AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDSET, 0);
        error = mAlsaControl->set("Voice Digital Loopback Volume",
                                AUDIO_CODEC_SIDETONE_GAIN_HANDSET, 0);
        error = mAlsaControl->set("Voice Digital Playback Filter Switch", 1, 0);
        error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                                AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDSET, 0);
        error = mAlsaControl->set("VRX to ARX2 digital mixing", "VRX to ARXL2/ARXR2");
    }

    return error;
}

status_t AudioModemAlsa::voiceCallCodecSetHandfree()
{
    status_t error = NO_ERROR;

    // Capture path
    error = mAlsaControl->set("Microphone Offset Cancellation Mode", "Voice");

    error = mAlsaControl->set("Analog Capture Main Mic Switch", 1, 0);
    error = mAlsaControl->set("Main Microphone Bias Switch", 1, 0);
    error = mAlsaControl->set("Analog Capture Volume",
                             AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_L, 0);
    error = mAlsaControl->set("Analog Capture Left Switch", 1, 0);
    error = mAlsaControl->set("TX2 Capture Route", "Analog");
    error = mAlsaControl->set("AVADC Clock Priority", "Voice high priority");
    error = mAlsaControl->set("ADC Voice Analog Left Capture Power", 1, 0);
    error = mAlsaControl->set("TX2 Digital Capture Volume",
                             AUDIO_CODEC_TXL2_VOL_HANDFREE, 0);
    error = mAlsaControl->set("Voice Digital Capture Left Filter Switch",
                             1, 0);

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        LOGV("dual mic. enabled");
        error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
        error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
        error = mAlsaControl->set("Analog Capture Volume",
                                 AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_R, 1);
        error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
        error = mAlsaControl->set("TX1 Capture Route", "Analog");
        error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                             AUDIO_CODEC_TXR2_VOL_HANDFREE, 1);
        error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                 1, 0);
    }

    // Playback path
    error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                             AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDFREE, 0);
    error = mAlsaControl->set("Voice Digital Loopback Volume",
                             AUDIO_CODEC_SIDETONE_GAIN_HANDFREE, 0);
    error = mAlsaControl->set("Voice Digital Playback Filter Switch", 1, 0);
    error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                             AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDFREE, 0);
    error = mAlsaControl->set("VRX to ARX2 digital mixing", "VRX to ARXL2/ARXR2");

    return error;
}

status_t AudioModemAlsa::voiceCallCodecSetHeadset()
{
    status_t error = NO_ERROR;

    // Capture path
    error = mAlsaControl->set("Microphone Offset Cancellation Mode", "Voice");

    error = mAlsaControl->set("Analog Capture Headset Mic Switch", 1, 0);
    error = mAlsaControl->set("Headset Microphone Bias Switch", 1, 0);
    error = mAlsaControl->set("Analog Capture Volume",
                             AUDIO_CODEC_ANAMIC_GAIN_HEADSET_L, 0);
    error = mAlsaControl->set("Analog Capture Left Switch", 1, 0);
    error = mAlsaControl->set("TX2 Capture Route", "Analog");
    error = mAlsaControl->set("AVADC Clock Priority", "Voice high priority");
    error = mAlsaControl->set("ADC Voice Analog Left Capture Power", 1, 0);
    error = mAlsaControl->set("TX2 Digital Capture Volume",
                             AUDIO_CODEC_TXL2_VOL_HEADSET, 0);
    error = mAlsaControl->set("Voice Digital Capture Left Filter Switch",
                             1, 0);

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        LOGV("dual mic. enabled");
        error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
        error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
        error = mAlsaControl->set("Analog Capture Volume",
                                 AUDIO_CODEC_ANAMIC_GAIN_HEADSET_R, 1);
        error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
        error = mAlsaControl->set("TX1 Capture Route", "Analog");
        error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                             AUDIO_CODEC_TXR2_VOL_HEADSET, 1);
        error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                 1, 0);
    }

    // Playback path
    error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                             AUDIO_CODEC_VOICE_DIGITAL_VOL_HEADSET, 0);
    error = mAlsaControl->set("Voice Digital Loopback Volume",
                             AUDIO_CODEC_SIDETONE_GAIN_HEADSET, 0);
    error = mAlsaControl->set("Voice Digital Playback Filter Switch", 1, 0);
    error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                             AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HEADSET, 0);
    error = mAlsaControl->set("VRX to ARX2 digital mixing", "VRX to ARXL2/ARXR2");

    return error;
}

#ifdef AUDIO_BLUETOOTH
status_t AudioModemAlsa::voiceCallCodecSetBluetooth()
{
    status_t error = NO_ERROR;

    error = voiceCallCodecBTPCMSet();

    return error;
}
#endif

status_t AudioModemAlsa::voiceCallCodecUpdateHandset()
{
    status_t error = NO_ERROR;

    // On Zoom2 the Headset mode doesn't exist only Handfree mode exist
    if (1) {
        error = voiceCallCodecUpdateHandfree();
    } else {
        // Capture path
        if (mPreviousAudioModemModes == AudioModemInterface::AUDIO_MODEM_HEADSET) {
            error = mAlsaControl->set("Analog Capture Headset Mic Switch", 0, 0);
            error = mAlsaControl->set("Headset Microphone Bias Switch", 0, 0);
            error = mAlsaControl->set("Analog Capture Main Mic Switch", 1, 0);
            error = mAlsaControl->set("Main Microphone Bias Switch", 1, 0);
#ifdef AUDIO_BLUETOOTH
        } else if (mPreviousAudioModemModes ==
                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
            error = voiceCallCodecBTPCMReset();

            error = voiceCallCodecSetHandset();

            return error;
#endif
        }

        error = mAlsaControl->set("Analog Capture Volume",
                                AUDIO_CODEC_ANAMIC_GAIN_HANDSET_L, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                                AUDIO_CODEC_TXL2_VOL_HANDSET, 0);

        if ((!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) &&
            (strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes"))) {
            LOGV("dual mic. enabled");
            error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
            error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
            error = mAlsaControl->set("Analog Capture Volume",
                                    AUDIO_CODEC_ANAMIC_GAIN_HANDSET_R, 1);
            error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
            error = mAlsaControl->set("TX1 Capture Route", "Analog");
            error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
            error = mAlsaControl->set("TX2 Digital Capture Volume",
                                AUDIO_CODEC_TXR2_VOL_HANDSET, 1);
            error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                    1, 0);
        }

        // Playback path
        error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                                AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDSET, 0);
        error = mAlsaControl->set("Voice Digital Loopback Volume",
                                AUDIO_CODEC_SIDETONE_GAIN_HANDSET, 0);
        error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                                AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDSET, 0);

    }

    return error;
}

status_t AudioModemAlsa::voiceCallCodecUpdateHandfree()
{
    status_t error = NO_ERROR;

    // Capture path
    if (mPreviousAudioModemModes == AudioModemInterface::AUDIO_MODEM_HEADSET) {
        error = mAlsaControl->set("Analog Capture Headset Mic Switch", 0, 0);
        error = mAlsaControl->set("Headset Microphone Bias Switch", 0, 0);
        error = mAlsaControl->set("Analog Capture Main Mic Switch", 1, 0);
        error = mAlsaControl->set("Main Microphone Bias Switch", 1, 0);
#ifdef AUDIO_BLUETOOTH
    } else if (mPreviousAudioModemModes ==
                AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        error = voiceCallCodecBTPCMReset();

        error = voiceCallCodecSetHandfree();

        return error;
#endif
    }

    error = mAlsaControl->set("Analog Capture Volume",
                            AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_L, 0);
    error = mAlsaControl->set("TX2 Digital Capture Volume",
                            AUDIO_CODEC_TXL2_VOL_HANDFREE, 0);

    if ((!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) &&
        (strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes"))) {
        LOGV("dual mic. enabled");
        error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
        error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
        error = mAlsaControl->set("Analog Capture Volume",
                                AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_R, 1);
        error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
        error = mAlsaControl->set("TX1 Capture Route", "Analog");
        error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                            AUDIO_CODEC_TXR2_VOL_HANDFREE, 1);
        error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                1, 0);
    }

    // Playback path
    error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                            AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDFREE, 0);
    error = mAlsaControl->set("Voice Digital Loopback Volume",
                            AUDIO_CODEC_SIDETONE_GAIN_HANDFREE, 0);
    error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                            AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDFREE, 0);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecUpdateHeadset()
{
   status_t error = NO_ERROR;

    // Capture path
    if ((mPreviousAudioModemModes == AudioModemInterface::AUDIO_MODEM_HANDSET) ||
        (mPreviousAudioModemModes == AudioModemInterface::AUDIO_MODEM_HANDFREE)) {
        error = mAlsaControl->set("Analog Capture Main Mic Switch", 0, 0);
        error = mAlsaControl->set("Main Microphone Bias Switch", 0, 0);
        error = mAlsaControl->set("Analog Capture Headset Mic Switch", 1, 0);
        error = mAlsaControl->set("Headset Microphone Bias Switch", 1, 0);
#ifdef AUDIO_BLUETOOTH
    } else if (mPreviousAudioModemModes ==
                    AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        error = voiceCallCodecBTPCMReset();

        error = voiceCallCodecSetHeadset();

        return error;
#endif
    }

    error = mAlsaControl->set("Analog Capture Volume",
                            AUDIO_CODEC_ANAMIC_GAIN_HEADSET_L, 0);
    error = mAlsaControl->set("TX2 Digital Capture Volume",
                            AUDIO_CODEC_TXL2_VOL_HEADSET, 0);

    if ((!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) &&
        (strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes"))) {
        LOGV("dual mic. enabled");
        error = mAlsaControl->set("Analog Capture Sub Mic Switch", 1, 0);
        error = mAlsaControl->set("Sub Microphone Bias Switch", 1, 0);
        error = mAlsaControl->set("Analog Capture Volume",
                                AUDIO_CODEC_ANAMIC_GAIN_HEADSET_R, 1);
        error = mAlsaControl->set("Analog Capture Right Switch", 1, 0);
        error = mAlsaControl->set("TX1 Capture Route", "Analog");
        error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
        error = mAlsaControl->set("TX2 Digital Capture Volume",
                            AUDIO_CODEC_TXR2_VOL_HEADSET, 1);
        error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                1, 0);
    }

    // Playback path
    error = mAlsaControl->set("DAC Voice Digital Downlink Volume",
                            AUDIO_CODEC_VOICE_DIGITAL_VOL_HEADSET, 0);
    error = mAlsaControl->set("Voice Digital Loopback Volume",
                            AUDIO_CODEC_SIDETONE_GAIN_HEADSET, 0);
    error = mAlsaControl->set("VRX to ARX Digital Downlink Volume",
                            AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HEADSET, 0);

    return error;
}

#ifdef AUDIO_BLUETOOTH
status_t AudioModemAlsa::voiceCallCodecUpdateBluetooth()
{
    status_t error = NO_ERROR;

    error = voiceCallCodecStop();

    error = voiceCallCodecBTPCMSet();

    return error;
}
#endif // AUDIO_BLUETOOTH

status_t AudioModemAlsa::voiceCallCodecUpdate()
{
    status_t error = NO_ERROR;

    LOGV("Update Audio Codec Voice call: %04x", mCurrentAudioModemModes);

    if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDSET) {
        error = voiceCallCodecUpdateHandset();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDFREE) {
        error = voiceCallCodecUpdateHandfree();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HEADSET) {
        error = voiceCallCodecUpdateHeadset();
    #ifdef AUDIO_BLUETOOTH
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        error = voiceCallCodecUpdateBluetooth();
    #endif
    } else {
        LOGE("Audio Modem mode not supported: %04x", mCurrentAudioModemModes);
        return INVALID_OPERATION;
    }

    error = voiceCallCodecPCMUpdate();

    return error;
}

status_t AudioModemAlsa::voiceCallCodecReset()
{
    status_t error = NO_ERROR;

    LOGV("Stop Audio Codec Voice call");

    error = voiceCallCodecPCMReset();
#ifdef AUDIO_BLUETOOTH
    error = voiceCallCodecBTPCMReset();
#endif

    error = voiceCallCodecStop();

    return error;
}

status_t AudioModemAlsa::voiceCallCodecStop()
{
    status_t error = NO_ERROR;

    // Capture path
    error = mAlsaControl->set("Analog Capture Volume", 0, 0);
    error = mAlsaControl->set("Analog Capture Volume", 0, 1);
    error = mAlsaControl->set("TX2 Digital Capture Volume", 0, 0);
    error = mAlsaControl->set("TX2 Digital Capture Volume", 0, 1);
    error = mAlsaControl->set("Microphone Offset Cancellation Mode",
                              "Audio left-right 1");

    error = mAlsaControl->set("Analog Capture Headset Mic Switch", 0, 0);
    error = mAlsaControl->set("Headset Microphone Bias Switch", 0, 0);

    error = mAlsaControl->set("Analog Capture Main Mic Switch", 0, 0);
    error = mAlsaControl->set("Main Microphone Bias Switch", 0, 0);
    error = mAlsaControl->set("Analog Capture Left Switch", 0, 0);
    error = mAlsaControl->set("ADC Voice Analog Left Capture Power", 0, 0);
    error = mAlsaControl->set("Voice Digital Capture Left Filter Switch",
                              0, 0);

    if (!strcmp(mDevicePropPrevious->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        LOGV("dual mic. enabled");
        error = mAlsaControl->set("Analog Capture Sub Mic Switch", 0, 0);
        error = mAlsaControl->set("Sub Microphone Bias Switch", 0, 0);
        error = mAlsaControl->set("Analog Capture Right Switch", 0, 0);
        error = mAlsaControl->set("ADC Voice Analog Right Capture Power", 1, 0);
        error = mAlsaControl->set("Voice Digital Capture Right Filter Switch",
                                  0, 0);
    }

    // Playback path
    error = mAlsaControl->set("DAC Voice Digital Downlink Volume", 0, 0);
    error = mAlsaControl->set("VRX to ARX Digital Downlink Volume", 0, 0);
    error = mAlsaControl->set("Voice Digital Loopback Volume", 0, 0);
    error = mAlsaControl->set("Voice Digital Playback Filter Switch", 0, 0);
    error = mAlsaControl->set("VRX to ARX2 digital mixing", "Off");

    return error;
}

status_t AudioModemAlsa::voiceCallModemSet()
{
    status_t error = NO_ERROR;

    LOGV("Start Audio Modem Voice call");

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        error = mModem->setModemRouting(mCurrentAudioModemModes,
               AudioModemInterface::PCM_16_KHZ);
    } else {
        error = mModem->setModemRouting(mCurrentAudioModemModes,
               AudioModemInterface::PCM_8_KHZ);
    }
    if (error < 0) {
        LOGE("Unable to set Modem Voice Call routing: %s", strerror(error));
        return error;
    }

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        error =  mModem->setModemVoiceCallMultiMic(
                    AudioModemInterface::MODEM_DOUBLE_MIC);

    } else {
        error =  mModem->setModemVoiceCallMultiMic(
                    AudioModemInterface::MODEM_SINGLE_MIC);
    }
    if (error < 0) {
        LOGE("Unable to set Modem Voice Call multimic.: %s", strerror(error));
        return error;
    }

    error =  mModem->OpenModemVoiceCallStream();
    if (error < 0) {
        LOGE("Unable to open Modem Voice Call stream: %s", strerror(error));
        return error;
    }

    return error;
}

status_t AudioModemAlsa::voiceCallModemUpdate()
{
    status_t error = NO_ERROR;

    LOGV("Update Audio Modem Voice call");

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        error = mModem->setModemRouting(mCurrentAudioModemModes,
               AudioModemInterface::PCM_16_KHZ);
    } else {
        error = mModem->setModemRouting(mCurrentAudioModemModes,
               AudioModemInterface::PCM_8_KHZ);
    }
    if (error < 0) {
        LOGE("Unable to set Modem Voice Call routing: %s", strerror(error));
        return error;
    }

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                "Yes")) {
        error =  mModem->setModemVoiceCallMultiMic(
                    AudioModemInterface::MODEM_DOUBLE_MIC);

    } else {
        error =  mModem->setModemVoiceCallMultiMic(
                    AudioModemInterface::MODEM_SINGLE_MIC);
    }
    if (error < 0) {
        LOGE("Unable to set Modem Voice Call multimic.: %s", strerror(error));
        return error;
    }

    return error;
}

status_t AudioModemAlsa::voiceCallModemReset()
{
    status_t error = NO_ERROR;

    LOGV("Stop Audio Modem Voice call");

    error = mModem->CloseModemVoiceCallStream();
    if (error < 0) {
        LOGE("Unable to close Modem Voice Call stream: %s", strerror(error));
        return error;
    }

    return error;
}

status_t AudioModemAlsa::voiceCallVolume(ALSAControl *alsaControl, float volume)
{
    status_t error = NO_ERROR;
    unsigned int setVolume;
    int i = 0;

    while(voiceCallVolumeProp[i].device) {
        voiceCallVolumeInfo *info = voiceCallVolumeProp[i].mInfo;

        // Make sure volume is between bounds.
        setVolume = info->min + volume * (info->max - info->min);
        if (setVolume > info->max) setVolume = info->max;
        if (setVolume < info->min) setVolume = info->min;

        LOGV("%s: in call volume level to apply: %d", voiceCallVolumeProp[i].volumeName,
                setVolume);

        error = alsaControl->set(voiceCallVolumeProp[i].volumeName, setVolume, 0);
        if (error < 0) {
            LOGE("%s: error applying in call volume: %d", voiceCallVolumeProp[i].volumeName,
                    setVolume);
            return error;
        }
        i++;
    }
    return NO_ERROR;
}
} // namespace android
