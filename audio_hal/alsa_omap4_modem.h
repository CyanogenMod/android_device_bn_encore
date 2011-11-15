/* alsa_omap4_modem.h
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

#ifndef ANDROID_ALSA_OMAP4_MODEM_H
#define ANDROID_ALSA_OMAP4_MODEM_H

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
//     sampleRate  : 8Khz, 16Khz, auto
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
    "auto"
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
// Preamplifier attenuation gain are
//  index       gain
//  1           0dB
//  0           6dB
#define AUDIO_CODEC_CAPTURE_PREAMP_ATT_HANDSET      1
#define AUDIO_CODEC_CAPTURE_PREAMP_ATT_HANDFREE     1
#define AUDIO_CODEC_CAPTURE_PREAMP_ATT_HEADSET      1

// amplifier volumes are
//  index       gain
//  0           6dB
//  1           12dB
//  2           18dB
//  3           24dB
//  4           30dB
#define AUDIO_CODEC_CAPTURE_VOL_HANDSET             3
#define AUDIO_CODEC_CAPTURE_VOL_HANDFREE            3
#define AUDIO_CODEC_CAPTURE_VOL_HEADSET             3

// amplifier volumes are
//  index       gain
//  0           -24dB
//  ...
//  15          6dB
//
//  step 2dB
#define AUDIO_CODEC_EARPIECE_GAIN        15

// amplifier volumes are
//  index       gain
//  0           -52dB
//  ...
//  29          6dB
//
//  step 2dB
#define AUDIO_CODEC_HANDFREE_GAIN        23

// amplifier volumes are
//  index       gain
//  0           -30dB
//  ...
//  15          0dB
//
//  step 2dB
#define AUDIO_CODEC_HEADSET_GAIN        15

// ABE Audio UL voice mixer Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_AUDUL_VOICE_VOL_HANDSET       120
#define AUDIO_ABE_AUDUL_VOICE_VOL_HANDFREE      120
#define AUDIO_ABE_AUDUL_VOICE_VOL_HEADSET       120
#define AUDIO_ABE_AUDUL_VOICE_VOL_BLUETOOTH     120

// DMIC ABE Uplink Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_DMIC_MAIN_UL_VOL_HANDFREE     140
#define AUDIO_ABE_DMIC_SUB_UL_VOL_HANDFREE      140
#define AUDIO_ABE_DMIC_MAIN_UL_VOL_HANDSET      140
#define AUDIO_ABE_DMIC_SUB_UL_VOL_HANDSET       140

// AMIC ABE Uplink Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_AMIC_UL_VOL_HANDFREE          120
#define AUDIO_ABE_AMIC_UL_VOL_HANDSET           120

// Bluetooth MIC ABE Uplink Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_BT_MIC_UL_VOL 120

// Sidetone Downlink Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_SIDETONE_DL_VOL_HANDSET       118
#define AUDIO_ABE_SIDETONE_DL_VOL_HANDFREE     0
#define AUDIO_ABE_SIDETONE_DL_VOL_HEADSET       118
#define AUDIO_ABE_SIDETONE_DL_VOL_BLUETOOTH     118

// Sidetone Uplink Volume
// Range values: min=0,max=149,step=1
// dBscale-min=-120.00dB,step=1.00dB,mute=0
#define AUDIO_ABE_SIDETONE_UL_VOL_HANDSET       90
#define AUDIO_ABE_SIDETONE_UL_VOL_HANDFREE      0
#define AUDIO_ABE_SIDETONE_UL_VOL_HEADSET       90
#define AUDIO_ABE_SIDETONE_UL_VOL_BLUETOOTH     90

// Modem interface static library to use
// are declared in the prop. key modem.audio.libpath
// if not found the generic library is used
#define AUDIO_MODEM_LIB_PATH_PROPERTY   "modem.audio.libpath"
#define AUDIO_MODEM_LIB_DEFAULT_PATH    "/system/lib/libaudiomodemgeneric.so"

// Audio ALSA PCM configuration
#define AUDIO_MODEM_PCM_HANDLE_NAME    "hw:0,5"
#define AUDIO_MODEM_PCM_LATENCY     500000

// Define priority order per device connection
// This is needed in case multipple device are connected
// lower value is the higher priority
// device can't have the same priority
#ifdef AUDIO_BLUETOOTH
#define AUDIO_MODEM_DEVICE_BLUETOOTH_PRIORITY    0
#define AUDIO_MODEM_DEVICE_HEADSET_PRIORITY      1
#define AUDIO_MODEM_DEVICE_HANDSET_PRIORITY      2
#define AUDIO_MODEM_DEVICE_HANDFREE_PRIORITY     3
#define AUDIO_MODEM_MAX_DEVICE    4
#else
#define AUDIO_MODEM_DEVICE_HEADSET_PRIORITY      0
#define AUDIO_MODEM_DEVICE_HANDSET_PRIORITY      1
#define AUDIO_MODEM_DEVICE_HANDFREE_PRIORITY     2
#define AUDIO_MODEM_MAX_DEVICE    3

#endif


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

struct voiceCallControlMainInfo
{
    bool        updateFlag;
    uint32_t    devices;
    int         mode;
    bool       multimediaUpdate;
};

struct voiceCallControlLocalInfo
{
    uint32_t    devices;
    int         mode;
    bool       multimediaUpdate;
    ALSAControl *mAlsaControl;
};

class AudioModemAlsa
{
public:
                AudioModemAlsa();
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
    status_t     voiceCallControls(uint32_t devices, int mode, bool multimediaUpdate);
    // Voice Call control thread
    void        voiceCallControlsThread(void);
    void        voiceCallControlsMutexLock(void);
    void        voiceCallControlsMutexUnlock(void);

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
    status_t     voiceCallCodecPCMReset(void);
    status_t     voiceCallCodecStop(void);

    status_t     voiceCallModemSet(void);
    status_t     voiceCallModemUpdate(void);
    status_t     voiceCallModemReset(void);

    status_t     voiceCallBTDeviceEnable(void);
    status_t     voiceCallBTDeviceDisable(void);

    status_t     voiceCallSidetoneSet(int mode);
    status_t     voiceCallSidetoneReset();

    #ifdef AUDIO_BLUETOOTH
        status_t     voiceCallCodecSetBluetooth(void);
        status_t     voiceCallCodecUpdateBluetooth(void);
        status_t     voiceCallCodecBTPCMSet(void);
        status_t     voiceCallCodecBTPCMReset(void);
    #endif
    status_t voiceCallVolume(ALSAControl *alsaControl, float volume);

    char        *mBoardName;
    int         mVoiceCallState;
    uint32_t    mCurrentAudioModemModes;
    uint32_t    mPreviousAudioModemModes;
    // list of properties per devices
    KeyedVector<uint32_t, AudioModemDeviceProperties *> mDevicePropList;
    AudioModemDeviceProperties *mDeviceProp;
    AudioModemDeviceProperties *mDevicePropPrevious;

    voiceCallControlMainInfo    mVoiceCallControlMainInfo;

    AudioModemInterface     *mModem;

    snd_pcm_t *pHandle;
    snd_pcm_t *cHandle;

    // DMIC/AMIC allocation
    status_t configMicrophones(void);
    status_t microphoneChosen(void);

    // Equalizer
    status_t configEqualizers(void);

    // Posix thread
    pthread_t       mVoiceCallControlThread;
    pthread_mutex_t mVoiceCallControlMutex;
    pthread_once_t  mVoiceCallControlKeyOnce;
    pthread_cond_t  mVoiceCallControlNewParams;

    // Multimedia update
    status_t     multimediaCodecUpdate(void);

    // Automatic sample rate
    uint32_t mVoiceCallSampleRate;

    // Audio device priority
    uint32_t mAudioDevicePriority[AUDIO_MODEM_MAX_DEVICE];
};
};        // namespace android
#endif    // ANDROID_ALSA_OMAP4_MODEM_H
