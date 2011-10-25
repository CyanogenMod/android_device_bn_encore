/* alsa_omap3_modem.h
 **
 ** Copyright (C) 2009-2010, Texas Instruments
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

#ifndef ANDROID_ALSA_OMAP3_MODEM_H
#define ANDROID_ALSA_OMAP3_MODEM_H

#include <utils/Errors.h>
#include <utils/KeyedVector.h>

namespace android
{
// The name of the audio modem properties keys is defined like below:
// Note: property key is limited to 32 characters
//
// modem.<audio_mode>.<stream>.<setting>
//
// with
//
// audio_mode:
//     hand for handset
//     free for handfree
//     head for headset
//     bt for bluetooth
//     aux for auxiliary
//
// stream:
//     voice for voice_call
//     audioi for audio_input
//     audioo for audio_output
//
// The list of the setting by order is:
//         Voicel Call multimic
//         Voicel Call sampleRate
//         Voicel Call nearEndVol
//         Voicel Call farEndVol
//
//         Audio Input sampleRate
//         Audio Input nearEndVol
//         Audio Input farEndVol
//         Audio Input channel
//         Audio Input recordType
//
//         Audio Output sampleRate
//         Audio Output nearEndVol
//         Audio Output farEndVol
//         Audio Output channel
//         Audio Output audioType

// Possible value for each settings:
//     multimic    : N, Y
//     sampleRate  : 8KHZ, 16KHZ
//     nearEndVol  : from 0.00 to 1.00 step 0.01
//     farEndVol   : from 0.00 to 1.00 step 0.01
//     channel     : I2S1, VOICE, VOICE_SECOND, VOICE_THIRD
//     audioType   : SOUND, SOUND_NEAREND_VOICE, SOUND_FAREND_VOICE
//     recordType  : NO_ECHO, ECHO
//

#define PROP_BASE_NAME "modem"

static const char *audioModeName[] = {
    ".hand",
    ".free",
    ".head",
    ".bt",
    ".aux",
};
static const int audioModeNameLen = (sizeof(audioModeName) / sizeof(char *));

static const char *modemStreamName[] = {
    ".voice",
    ".audioi",
    ".audioo"
};
static const int modemStreamNameLen = (sizeof(modemStreamName) / sizeof(char *));

#define VOICE_CALL_STREAM   (1<<0)
#define AUDIO_INPUT_STREAM  (1<<1)
#define AUDIO_OUTPUT_STREAM (1<<2)

#define AUDIO_MODEM_SETTING_DEFAULT "default"

#define AUDIO_MODEM_SETTING_MAX 14
#define AUDIO_MODEM_SETTING_LEN 15

#define AUDIO_MODEM_VOICE_CALL_MULTIMIC    0
#define AUDIO_MODEM_VOICE_CALL_SAMPLERATE  1
#define AUDIO_MODEM_VOICE_CALL_NEARENDVOL  2
#define AUDIO_MODEM_VOICE_CALL_FARENDVOL   3
#define AUDIO_MODEM_AUDIO_IN_SAMPLERATE    4
#define AUDIO_MODEM_AUDIO_IN_NEARENDVOL    5
#define AUDIO_MODEM_AUDIO_IN_FARENDVOL     6
#define AUDIO_MODEM_AUDIO_IN_CHANNEL       7
#define AUDIO_MODEM_AUDIO_IN_RECORDTYPE    8
#define AUDIO_MODEM_AUDIO_OUT_SAMPLERATE   9
#define AUDIO_MODEM_AUDIO_OUT_NEARENDVOL   10
#define AUDIO_MODEM_AUDIO_OUT_FARENDVOL    11
#define AUDIO_MODEM_AUDIO_OUT_CHANNEL      12
#define AUDIO_MODEM_AUDIO_OUT_AUDIOTYPE    13

struct settingString_t {
    char name[AUDIO_MODEM_SETTING_LEN];
};

struct settingNameperStream_t {
    const char *name;
    const char  streamUsed;
};

static settingNameperStream_t settingName[] = {

    { ".multimic",      (VOICE_CALL_STREAM) },
    { ".sampleRate",    (VOICE_CALL_STREAM | AUDIO_INPUT_STREAM | AUDIO_OUTPUT_STREAM) },
    { ".nearEndVol",    (VOICE_CALL_STREAM | AUDIO_INPUT_STREAM | AUDIO_OUTPUT_STREAM) },
    { ".farEndVol",     (VOICE_CALL_STREAM | AUDIO_INPUT_STREAM | AUDIO_OUTPUT_STREAM) },
    { ".channel",       (AUDIO_INPUT_STREAM | AUDIO_OUTPUT_STREAM) },
    { ".audioType",     (AUDIO_OUTPUT_STREAM) },
    { ".recordType",    (AUDIO_INPUT_STREAM) }
};
static const int settingNameLen = (sizeof(settingName) / sizeof(settingNameperStream_t));

//     multimic    : No, Yes
//     sampleRate  : 8Khz, 16Khz
//     nearEndVol  : from 0.00 to 1.00 step 0.01
//     farEndVol   : from 0.00 to 1.00 step 0.01
//     channel     : I2S1, Voice, Voice_second, Voice_third
//     audioType   : Sound, Nearend_voice, Farend_voice
//     recordType  : No_echo, Echo

static const char *multimicValue[] = {
    "No",
    "Yes"
};
static const int multimicValueLen = (sizeof(multimicValue) / sizeof(char *));

static const char *sampleRateValue[] = {
    "8Khz",
    "16Khz"
};
static const int sampleRateValueLen = (sizeof(sampleRateValue) / sizeof(char *));

static const char *channelValue[] = {
    "I2S1",
    "Voice",
    "Voice_second",
    "Voice_third"
};
static const int channelValueLen = (sizeof(channelValue) / sizeof(char *));

static const char *recordTypeValue[] = {
    "Sound",
    "Nearend_voice",
    "Farend_voice"
};
static const int recordTypeValueLen = (sizeof(recordTypeValue) / sizeof(char *));

//
// Audio Codec fine tuning paratemers:
//
// TODO put in property keys
#define AUDIO_CODEC_ANAMIC_GAIN_HANDSET_L 3
#define AUDIO_CODEC_ANAMIC_GAIN_HANDSET_R 3
#define AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_L 3
#define AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_R 3
#define AUDIO_CODEC_ANAMIC_GAIN_HEADSET_L 3
#define AUDIO_CODEC_ANAMIC_GAIN_HEADSET_R 3

#define AUDIO_CODEC_TXL2_VOL_HANDSET 16
#define AUDIO_CODEC_TXR2_VOL_HANDSET 16
#define AUDIO_CODEC_TXL2_VOL_HANDFREE 16
#define AUDIO_CODEC_TXR2_VOL_HANDFREE 16
#define AUDIO_CODEC_TXL2_VOL_HEADSET 16
#define AUDIO_CODEC_TXR2_VOL_HEADSET 16

#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDSET 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDFREE 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HEADSET 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_BLUETOOTH 12

#define AUDIO_CODEC_SIDETONE_GAIN_HANDSET 20
#define AUDIO_CODEC_SIDETONE_GAIN_HANDFREE 0
#define AUDIO_CODEC_SIDETONE_GAIN_HEADSET 20
#define AUDIO_CODEC_SIDETONE_GAIN_BLUETOOTH 20

#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDSET 13
#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDFREE 13
#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HEADSET 13

#define AUDIO_CODEC_VOICE_DIGITAL_CAPTURE_VOL_BLUETOOTH 12


// Modem interface static library to use
// are declared in the prop. key modem.audio.libpath
// if not found the generic library is used
#define AUDIO_MODEM_LIB_PATH_PROPERTY   "modem.audio.libpath"
#define AUDIO_MODEM_LIB_DEFAULT_PATH    "/system/lib/libaudiomodemgeneric.so"

// Voice Call Volume
struct voiceCallVolumeInfo
{
    unsigned int      min;
    unsigned int      max;
};

struct voiceCallVolumeList
{
    const uint32_t  device;
    const char      *volumeName;
    voiceCallVolumeInfo *mInfo;
};

class AudioModemAlsa
{
public:
                AudioModemAlsa(ALSAControl *alsaControl);
    virtual    ~AudioModemAlsa();

    class AudioModemDeviceProperties
    {
    public:
        AudioModemDeviceProperties(const char *audioModeName);

        void initProperties(int index,
                            char *propValue,
                            bool def);

        // The list of the setting by order is:
        // Voicel Call multimic
        // Voicel Call sampleRate
        // Voicel Call nearEndVol
        // Voicel Call farEndVol
        //
        // Audio Input sampleRate
        // Audio Input nearEndVol
        // Audio Input farEndVol
        // Audio Input channel
        // Audio Input recordType
        //
        // Audio Output sampleRate
        // Audio Output nearEndVol
        // Audio Output farEndVol
        // Audio Output channel
        // Audio Output audioType
        struct settingString_t  settingsList[AUDIO_MODEM_SETTING_MAX];

        uint32_t audioMode;
    };

    enum audio_modem_voice_call_state {
        AUDIO_MODEM_VOICE_CALL_OFF = 0,
        AUDIO_MODEM_VOICE_CALL_ON,
        AUDIO_MODEM_VOICE_CALL_STATE_INVALID
    };


    AudioModemInterface *create(void);
    status_t     audioModemSetProperties(void);
    status_t     voiceCallControls(uint32_t devices, int mode,
                                    ALSAControl *alsaControl);
    status_t     setCurrentAudioModemModes(uint32_t devices);

    status_t     voiceCallCodecSet(void);
    status_t     voiceCallCodecUpdate(void);
    status_t     voiceCallCodecReset(void);
    status_t     voiceCallCodecSetHandset(void);
    status_t     voiceCallCodecSetHandfree(void);
    status_t     voiceCallCodecSetHeadset(void);
    status_t     voiceCallCodecUpdateHandset(void);
    status_t     voiceCallCodecUpdateHandfree(void);
    status_t     voiceCallCodecUpdateHeadset(void);
    status_t     voiceCallCodecPCMSet(void);
    status_t     voiceCallCodecPCMUpdate(void);
    status_t     voiceCallCodecPCMReset(void);
    status_t     voiceCallCodecStop(void);

    status_t     voiceCallModemSet(void);
    status_t     voiceCallModemUpdate(void);
    status_t     voiceCallModemReset(void);

    status_t     voiceCallBTDeviceEnable(void);
    status_t     voiceCallBTDeviceDisable(void);
    #ifdef AUDIO_BLUETOOTH
        status_t     voiceCallCodecSetBluetooth(void);
        status_t     voiceCallCodecUpdateBluetooth(void);
        status_t     voiceCallCodecBTPCMSet(void);
        status_t     voiceCallCodecBTPCMReset(void);
    #endif
    status_t voiceCallVolume(ALSAControl *alsaControl, float volume);

    char        *mBoardName;
    ALSAControl *mAlsaControl;
    int         mVoiceCallState;
    uint32_t    mCurrentAudioModemModes;
    uint32_t    mPreviousAudioModemModes;
    // list of properties per devices
    KeyedVector<uint32_t, AudioModemDeviceProperties *> mDevicePropList;
    AudioModemDeviceProperties *mDeviceProp;
    AudioModemDeviceProperties *mDevicePropPrevious;

    AudioModemInterface     *mModem;
};
};        // namespace android
#endif    // ANDROID_ALSA_OMAP3_MODEM_H
