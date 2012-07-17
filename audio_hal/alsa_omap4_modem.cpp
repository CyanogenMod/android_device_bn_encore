/* alsa_omap4_modem.cpp
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

#define LOG_TAG "Omap4ALSAModem"

#include <utils/Log.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <pthread.h>

#include "AudioHardwareALSA.h"
#include <media/AudioRecord.h>
#include "alsa_omap4.h"

namespace android
{
// ----------------------------------------------------------------------------
// Voice call volume
#define VOICE_CALL_VOLUME_PROP(dev, name) \
    {\
        dev, name, NULL\
    }

static voiceCallVolumeList
voiceCallVolumeProp[] = {
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HANDSET,
                            "DL1 Voice Playback Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HANDFREE,
                            "DL2 Voice Playback Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_HEADSET,
                            "DL1 Voice Playback Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_AUX,
                            "DL1 Voice Playback Volume"),
    VOICE_CALL_VOLUME_PROP(AudioModemInterface::AUDIO_MODEM_BLUETOOTH,
                            "DL1 Voice Playback Volume"),
    VOICE_CALL_VOLUME_PROP(0, "")
};

#define WORKAROUND_AVOID_VOICE_VOLUME_MAX   1
#define WORKAROUND_MAX_VOICE_VOLUME         120
#define WORKAROUND_AVOID_VOICE_VOLUME_MIN   1
#define WORKAROUND_MIN_VOICE_VOLUME         90

// Pointer to Thread local info
voiceCallControlLocalInfo    *mInfo;

// Properties manager
Omap4ALSAManager propModemMgr;
// ----------------------------------------------------------------------------
#define CHECK_ERROR(func, error)   if ((error = func) != NO_ERROR) { \
                                ALOGV("Error %s on %s line %d", strerror(error), \
                                     __FUNCTION__, __LINE__); \
                                return error; \
                            }
#define mAlsaControl mInfo->mAlsaControl
// ----------------------------------------------------------------------------
#if !LOG_NDEBUG
// Mutex counter for debug
static int voiceCallControlMutexCount = 0;
#endif
extern "C"
{
    void *VoiceCallControlsThreadStartup(void *_mAudioModemAlsa) {
        ALOGV("%s",__FUNCTION__);
        AudioModemAlsa *mAudioModemAlsa = (AudioModemAlsa * )_mAudioModemAlsa;
        mAudioModemAlsa->voiceCallControlsThread();
        delete mAudioModemAlsa;
        return (NULL);
    }

    static pthread_key_t   mVoiceCallControlThreadKey;
    void voiceCallControlInitThreadOnce(void)
    {
        status_t error = NO_ERROR;
        ALOGV("%s",__FUNCTION__);
        error = pthread_key_create(&mVoiceCallControlThreadKey,
                                    NULL);
        if (error != NO_ERROR) {
            ALOGE("Can't create the Voice Call Control Thread key");
            exit(error);
        }
    }
}
// ----------------------------------------------------------------------------
AudioModemAlsa::AudioModemAlsa()
{
    status_t error;
    pthread_attr_t  mVoiceCallControlAttr;

    ALOGV("Build date: %s time: %s", __DATE__, __TIME__);

    ALOGD("Initializing devices for Modem OMAP4 ALSA module");

    audioModemSetProperties();
    mModem = create();
    if (mModem) {
        error = mModem->initCheck() ;
        if (error != NO_ERROR) {
            ALOGE("Audio Modem Interface was not correctly initialized.");
            delete mModem;
            exit(error);
        }
    }
    else {
        ALOGE("No Audio Modem Interface found.");
        exit(-1);
    }
    mVoiceCallState = AUDIO_MODEM_VOICE_CALL_OFF;

    // Initialize Min and Max volume
    int i = 0;
    while(voiceCallVolumeProp[i].device) {
        voiceCallVolumeInfo *info = voiceCallVolumeProp[i].mInfo = new voiceCallVolumeInfo;

#if WORKAROUND_AVOID_VOICE_VOLUME_MAX
        ALOGV("Workaround: Voice call max volume name %s limited to: %d",
                voiceCallVolumeProp[i].volumeName, WORKAROUND_MAX_VOICE_VOLUME);
        info->max = WORKAROUND_MAX_VOICE_VOLUME;
#else
        error = alsaControl->getmax(voiceCallVolumeProp[i].volumeName, info->max);
#endif
#if WORKAROUND_AVOID_VOICE_VOLUME_MIN
        ALOGV("Workaround: Voice call min volume name %s limited to: %d",
                voiceCallVolumeProp[i].volumeName, WORKAROUND_MIN_VOICE_VOLUME);
        info->min = WORKAROUND_MIN_VOICE_VOLUME;
#else
        error = alsaControl->getmin(voiceCallVolumeProp[i].volumeName, info->min);
#endif
        ALOGV("Voice call volume name: %s min: %d max: %d", voiceCallVolumeProp[i].volumeName,
                 info->min, info->max);

        if (error != NO_ERROR) {
            ALOGE("Audio Voice Call volume was not correctly initialized.");
            delete mModem;
            exit(error);
        }
        i++;
    }

    // Initialize voice call control key once
    mVoiceCallControlKeyOnce = PTHREAD_ONCE_INIT;

    // Initialize voice call control mutex
    pthread_mutex_init(&mVoiceCallControlMutex, NULL);

    // Initialize voice call control param condition
    // This is used to wake up Voice call control thread when
    // new param is available
    pthread_cond_init(&mVoiceCallControlNewParams, NULL);

    // Initialize and set thread detached attribute
    pthread_attr_init(&mVoiceCallControlAttr);
    pthread_attr_setdetachstate(&mVoiceCallControlAttr, PTHREAD_CREATE_JOINABLE);

    voiceCallControlsMutexLock();
    mVoiceCallControlMainInfo.updateFlag = false;
    voiceCallControlsMutexUnlock();

    // Create thread for voice control
    error = pthread_create(&mVoiceCallControlThread, &mVoiceCallControlAttr,
                            VoiceCallControlsThreadStartup, (void *)this);
    if (error != NO_ERROR) {
        ALOGE("Error creating mVoiceCallControlThread");
        delete mModem;
        exit(error);
    }
    pthread_attr_destroy(&mVoiceCallControlAttr);

    // Properties manager init
    propModemMgr = Omap4ALSAManager();

    // initialize all equalizers to flat response for voice call.
    propModemMgr.set((String8)Omap4ALSAManager::DL2L_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[0]);
    propModemMgr.set((String8)Omap4ALSAManager::DL2R_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[0]);
    propModemMgr.set((String8)Omap4ALSAManager::DL1_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[0]);
    propModemMgr.set((String8)Omap4ALSAManager::AMIC_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    propModemMgr.set((String8)Omap4ALSAManager::DMIC_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    propModemMgr.set((String8)Omap4ALSAManager::SDT_EQ_PROFILE,
                        (String8)Omap4ALSAManager::EqualizerProfileList[0]);

    // initialize priority table
    mAudioDevicePriority[AUDIO_MODEM_DEVICE_HANDFREE_PRIORITY] =
                                AudioModemInterface::AUDIO_MODEM_HANDFREE;
    mAudioDevicePriority[AUDIO_MODEM_DEVICE_HANDSET_PRIORITY] =
                                AudioModemInterface::AUDIO_MODEM_HANDSET;
    mAudioDevicePriority[AUDIO_MODEM_DEVICE_HEADSET_PRIORITY] =
                                AudioModemInterface::AUDIO_MODEM_HEADSET;
#ifdef AUDIO_BLUETOOTH
    mAudioDevicePriority[AUDIO_MODEM_DEVICE_BLUETOOTH_PRIORITY] =
                                AudioModemInterface::AUDIO_MODEM_BLUETOOTH;
#endif
}

AudioModemAlsa::~AudioModemAlsa()
{
    ALOGD("Destroy devices for Modem OMAP4 ALSA module");
    mDevicePropList.clear();
    if (mModem) delete mModem;

    pthread_mutex_destroy(&mVoiceCallControlMutex);
    pthread_kill(mVoiceCallControlThread, SIGKILL);
}

AudioModemInterface* AudioModemAlsa::create()
{
    void *dlHandle;
    char libPath[PROPERTY_VALUE_MAX];
    AudioModemInterface *(*audioModem)(void);

    property_get(AUDIO_MODEM_LIB_PATH_PROPERTY,
                 libPath,
                 AUDIO_MODEM_LIB_DEFAULT_PATH);

    ALOGW_IF(!strcmp(libPath, AUDIO_MODEM_LIB_DEFAULT_PATH),
                    "Use generic Modem interface");

    dlHandle = dlopen(libPath, RTLD_NOW);
    if (dlHandle == NULL) {
        ALOGE("Audio Modem %s dlopen failed: %s", libPath, dlerror());
        exit(-1);
    }

    audioModem = (AudioModemInterface *(*)(void))dlsym(dlHandle, "createAudioModemInterface");
    if (audioModem == NULL) {
        ALOGE("createAudioModemInterface function not defined or not exported in %s", libPath);
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

                    ALOGV("%s = %s", propKey,
                                    deviceProp->settingsList[index].name);
                    index++;
                }
            }
        }
        mDevicePropList.add(deviceProp->audioMode, deviceProp);
    }
    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallControls(uint32_t devices, int mode, bool multimediaUpdate)
{
    ALOGV("%s: devices %04x mode %d MultimediaUpdate %d", __FUNCTION__, devices, mode, multimediaUpdate);

    // Ignore input devices
    if (!(devices & AudioSystem::DEVICE_IN_ALL)) {
        if ((mVoiceCallControlMainInfo.devices != devices) ||
            (mVoiceCallControlMainInfo.mode != mode) ||
            (multimediaUpdate)) {
            voiceCallControlsMutexLock();
            mVoiceCallControlMainInfo.devices = devices;
            mVoiceCallControlMainInfo.mode = mode;
            mVoiceCallControlMainInfo.updateFlag = true;
            mVoiceCallControlMainInfo.multimediaUpdate = multimediaUpdate;
            voiceCallControlsMutexUnlock();
            pthread_cond_signal(&mVoiceCallControlNewParams);
        }
    }
    return NO_ERROR;
}

void AudioModemAlsa::voiceCallControlsMutexLock(void)
{
    ALOGV("%s: %d", __FUNCTION__, ++voiceCallControlMutexCount);
    pthread_mutex_lock(&mVoiceCallControlMutex);
}

void AudioModemAlsa::voiceCallControlsMutexUnlock(void)
{
    ALOGV("%s: %d", __FUNCTION__, --voiceCallControlMutexCount);
    pthread_mutex_unlock(&mVoiceCallControlMutex);
}

void AudioModemAlsa::voiceCallControlsThread(void)
{
    status_t error = NO_ERROR;
    ALSAControl alsaControl("hw:00");

    error = pthread_once(&mVoiceCallControlKeyOnce,
                           voiceCallControlInitThreadOnce);
    if (error != NO_ERROR) {
        ALOGE("Error in voiceCallControlInitThreadOnce");
        goto exit;
    }

    mInfo = (voiceCallControlLocalInfo *)malloc(sizeof(voiceCallControlLocalInfo));
    if (mInfo == NULL) {
        ALOGE("Error allocating mVoiceCallControlThreadInfo");
        error = NO_MEMORY;
        goto exit;
    }

    error = pthread_setspecific(mVoiceCallControlThreadKey, mInfo);
    if (error != NO_ERROR) {
        ALOGE("Error in Voice Call info set specific");
        goto exit;
    }

    mInfo->devices = 0;
    mInfo->mode = AudioSystem::MODE_INVALID;
    mAlsaControl = &alsaControl;

    for (;;) {
        voiceCallControlsMutexLock();
        // Wait for new parameters
        if (!mVoiceCallControlMainInfo.updateFlag) {
            pthread_cond_wait(&mVoiceCallControlNewParams, &mVoiceCallControlMutex);
        }
        mVoiceCallControlMainInfo.updateFlag = false;
        mInfo = (voiceCallControlLocalInfo *)pthread_getspecific(mVoiceCallControlThreadKey);
        if (mInfo == NULL) {
            ALOGE("Error in Voice Call info get specific");
            goto exit;
        }
        mInfo->devices = mVoiceCallControlMainInfo.devices;
        mInfo->mode = mVoiceCallControlMainInfo.mode;
        mInfo->multimediaUpdate = mVoiceCallControlMainInfo.multimediaUpdate;
        voiceCallControlsMutexUnlock();
        ALOGV("%s: devices %04x mode %d forceUpdate %d", __FUNCTION__, mInfo->devices, mInfo->mode,
                                                                    mInfo->multimediaUpdate);

        // Check mics config
        error = microphoneChosen();
        if (error < 0) goto exit;

        if ((mInfo->mode == AudioSystem::MODE_IN_CALL) &&
            (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_OFF)) {
            // enter in voice call mode
            error = setCurrentAudioModemModes(mInfo->devices);
            if (error < 0) goto exit;
            mDeviceProp = mDevicePropList.valueFor(mCurrentAudioModemModes);
            error = voiceCallModemSet();
            if (error < 0) goto exit;
            voiceCallControlsMutexLock();
            error = voiceCallCodecSet();
            voiceCallControlsMutexUnlock();
            if (error < 0) goto exit;
            mVoiceCallState = AUDIO_MODEM_VOICE_CALL_ON;
        } else if ((mInfo->mode == AudioSystem::MODE_IN_CALL) &&
                (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_ON)) {
            // update in voice call mode
            mPreviousAudioModemModes = mCurrentAudioModemModes;
            error = setCurrentAudioModemModes(mInfo->devices);
            if (error < 0) {
                mCurrentAudioModemModes = mPreviousAudioModemModes;
                goto exit;
            }
            mDevicePropPrevious = mDeviceProp;
            mDeviceProp = mDevicePropList.valueFor(mCurrentAudioModemModes);
            if (mCurrentAudioModemModes != mPreviousAudioModemModes) {
                error = voiceCallModemUpdate();
                if (error < 0) goto exit;
                voiceCallControlsMutexLock();
                error = voiceCallCodecUpdate();
                voiceCallControlsMutexUnlock();
                if (error < 0) goto exit;
            } else if (mInfo->multimediaUpdate) {
                voiceCallControlsMutexLock();
                error = multimediaCodecUpdate();
                voiceCallControlsMutexUnlock();
                if (error < 0) goto exit;
            } else {
                ALOGI("Audio Modem Mode doesn't changed: no update needed");
            }
        } else if ((mInfo->mode != AudioSystem::MODE_IN_CALL) &&
                (mVoiceCallState == AUDIO_MODEM_VOICE_CALL_ON)) {
            // we just exit voice call mode
            mPreviousAudioModemModes = 0;
            mCurrentAudioModemModes = 0;
            error = voiceCallModemReset();
            if (error < 0) goto exit;
            voiceCallControlsMutexLock();
            error = voiceCallCodecReset();
            voiceCallControlsMutexUnlock();
            if (error < 0) goto exit;
            mVoiceCallState = AUDIO_MODEM_VOICE_CALL_OFF;
        }
    } // for(;;)

exit:
    ALOGE("%s: exit with error %d (%s)", __FUNCTION__, error, strerror(error));
    pthread_exit((void*) error);
}

status_t AudioModemAlsa::setCurrentAudioModemModes(uint32_t devices)
{
    int i;
    uint32_t nextAudioMode = 0;

    for (i = 0; i < AUDIO_MODEM_MAX_DEVICE; i++) {
        if (devices & mAudioDevicePriority[i]) {
            nextAudioMode = mCurrentAudioModemModes = mAudioDevicePriority[i];
            ALOGV("New Audio Modem Modes: %04x", mCurrentAudioModemModes);
            return NO_ERROR;
        }
    }
    if (!nextAudioMode) {
        ALOGE("Devices %04x not supported...", devices);
        if (!mCurrentAudioModemModes) {
            ALOGE("No current devices switch to AUDIO_MODEM_HANDSET...");
            mCurrentAudioModemModes = AudioModemInterface::AUDIO_MODEM_HANDSET;
        } else {
            ALOGE("Stay on the current devices: %04x...", mCurrentAudioModemModes);
        }
        return NO_ERROR;
    }

    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallCodecSet()
{
    status_t error = NO_ERROR;

    ALOGV("Start Audio Codec Voice call: %04x", mCurrentAudioModemModes);

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
        ALOGE("Audio Modem mode not supported: %04x", mCurrentAudioModemModes);
        return INVALID_OPERATION;
    }

    error = voiceCallCodecPCMSet();
    error = voiceCallSidetoneSet(mCurrentAudioModemModes);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecPCMSet()
{
    int error;
    unsigned int sampleRate;

    if ((error = snd_pcm_open(&cHandle,
                    AUDIO_MODEM_PCM_HANDLE_NAME,
                    SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        ALOGE("Modem PCM capture open error: %d:%s", error, snd_strerror(error));
        return INVALID_OPERATION;
    }
    ALOGV("snd_pcm_open(%p, %s, SND_PCM_STREAM_CAPTURE, 0)", cHandle, AUDIO_MODEM_PCM_HANDLE_NAME);

    if ((error = snd_pcm_open(&pHandle,
                    AUDIO_MODEM_PCM_HANDLE_NAME,
                    SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        ALOGE("Modem PCM playback open error: %d:%s", error, snd_strerror(error));
        return INVALID_OPERATION;
    }
    ALOGV("snd_pcm_open(%p, %s, SND_PCM_STREAM_PLAYBACK, 0)", pHandle, AUDIO_MODEM_PCM_HANDLE_NAME);

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "auto")) {
        if (mVoiceCallSampleRate == AudioModemInterface::PCM_16_KHZ) {
            sampleRate = 16000;
        } else {
            sampleRate = 8000;
        }
    }
    else if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        sampleRate = 16000;
    } else {
        sampleRate = 8000;
    }

    if ((error = snd_pcm_set_params(cHandle,
                    SND_PCM_FORMAT_S16_LE,
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    2,
                    sampleRate,
                    1,
                    AUDIO_MODEM_PCM_LATENCY)) < 0) {
        ALOGE("Modem PCM capture params error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }
    if ((error = snd_pcm_set_params(pHandle,
                    SND_PCM_FORMAT_S16_LE,
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    2,
                    sampleRate,
                    1,
                    AUDIO_MODEM_PCM_LATENCY)) < 0) {
        ALOGE("Modem PCM playback params error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }

    if ((error = snd_pcm_start(cHandle)) < 0) {
        ALOGE("Modem PCM capture start error: %d:%s", error, snd_strerror(error));
        return INVALID_OPERATION;
    }
    if ((error = snd_pcm_start(pHandle)) < 0) {
        ALOGE("Modem PCM playback start error: %d:%s", error, snd_strerror(error));
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallCodecPCMReset()
{
    int error;

    if ((error = snd_pcm_drop(cHandle)) < 0) {
        ALOGE("Modem PCM capture drop error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }
    if ((error = snd_pcm_drop(pHandle)) < 0) {
        ALOGE("Modem PCM playback drop error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }

    if ((error = snd_pcm_close(cHandle)) < 0) {
        ALOGE("Modem PCM capture close error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }
    ALOGV("snd_pcm_close(%p)", cHandle);

    if ((error = snd_pcm_close(pHandle)) < 0) {
        ALOGE("Modem PCM playback close error: %s", snd_strerror(error));
        return INVALID_OPERATION;
    }
    ALOGV("snd_pcm_close(%p)", pHandle);

    return NO_ERROR;
}

status_t AudioModemAlsa::voiceCallCodecSetHandset()
{
    status_t error = NO_ERROR;
    String8 keyMain = (String8)Omap4ALSAManager::MAIN_MIC;
    String8 keySub = (String8)Omap4ALSAManager::SUB_MIC;
    String8 main;
    String8 sub;

    // Enable Playback voice path
    CHECK_ERROR(mAlsaControl->set("EP Playback", "On"), error);
    CHECK_ERROR(mAlsaControl->set("Earphone Playback Volume", AUDIO_CODEC_EARPIECE_GAIN), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mixer Voice", 1), error);
    CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Playback", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 PDM Switch", 1), error);
    CHECK_ERROR(mAlsaControl->set("SDT DL Volume",
                                AUDIO_ABE_SIDETONE_DL_VOL_HANDSET, -1), error);
    // Enable Capture voice path
    CHECK_ERROR(propModemMgr.get(keyMain, main), error);
    CHECK_ERROR(propModemMgr.get(keySub, sub), error);
    if ((strncmp(main.string(), "A", 1) == 0) ||
        (strncmp(sub.string(), "A", 1) == 0)) {
        CHECK_ERROR(mAlsaControl->set("Analog Left Capture Route", "Main Mic"), error);
        if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) {
            ALOGV("dual mic. enabled");

            // Enable Sub mic
            CHECK_ERROR(mAlsaControl->set("Analog Right Capture Route", "Sub Mic"),
                                            error);
        }
        CHECK_ERROR(mAlsaControl->set("Capture Preamplifier Volume",
                                    AUDIO_CODEC_CAPTURE_PREAMP_ATT_HANDSET, -1), error);
        CHECK_ERROR(mAlsaControl->set("Capture Volume",
                                    AUDIO_CODEC_CAPTURE_VOL_HANDSET, -1), error);
    }
    CHECK_ERROR(mAlsaControl->set("AUDUL Voice UL Volume",
                                AUDIO_ABE_AUDUL_VOICE_VOL_HANDSET, -1), error);
    CHECK_ERROR(configMicrophones(), error);
    CHECK_ERROR(configEqualizers(), error);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecSetHandfree()
{
    status_t error = NO_ERROR;
    String8 keyMain = (String8)Omap4ALSAManager::MAIN_MIC;
    String8 keySub = (String8)Omap4ALSAManager::SUB_MIC;
    String8 main;
    String8 sub;

    // Enable Playback voice path
    CHECK_ERROR(mAlsaControl->set("HF Left Playback", "HF DAC"), error);
    CHECK_ERROR(mAlsaControl->set("HF Right Playback", "HF DAC"), error);
    CHECK_ERROR(mAlsaControl->set("Handsfree Playback Volume", AUDIO_CODEC_HANDFREE_GAIN), error);
    CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL2 Mixer Voice", 1), error);
    CHECK_ERROR(mAlsaControl->set("SDT DL Volume",
                                AUDIO_ABE_SIDETONE_DL_VOL_HANDFREE, -1), error);

    // Enable Capture voice path
    CHECK_ERROR(propModemMgr.get(keyMain, main), error);
    CHECK_ERROR(propModemMgr.get(keySub, sub), error);
    if ((strncmp(main.string(), "A", 1) == 0) ||
        (strncmp(sub.string(), "A", 1) == 0)) {
        CHECK_ERROR(mAlsaControl->set("Analog Left Capture Route", "Main Mic"), error);
        if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) {
            ALOGV("dual mic. enabled");

            // Enable Sub mic
            CHECK_ERROR(mAlsaControl->set("Analog Right Capture Route", "Sub Mic"),
                                            error);
        }
        CHECK_ERROR(mAlsaControl->set("Capture Preamplifier Volume",
                                    AUDIO_CODEC_CAPTURE_PREAMP_ATT_HANDFREE, -1), error);
        CHECK_ERROR(mAlsaControl->set("Capture Volume",
                                    AUDIO_CODEC_CAPTURE_VOL_HANDFREE, -1), error);
    }
    CHECK_ERROR(mAlsaControl->set("AUDUL Voice UL Volume",
                                AUDIO_ABE_AUDUL_VOICE_VOL_HANDFREE, -1), error);
    CHECK_ERROR(configMicrophones(), error);
    CHECK_ERROR(configEqualizers(), error);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecSetHeadset()
{
    status_t error = NO_ERROR;

    // Enable Playback voice path
    CHECK_ERROR(mAlsaControl->set("HS Left Playback", "HS DAC"), error);
    CHECK_ERROR(mAlsaControl->set("HS Right Playback", "HS DAC"), error);
    CHECK_ERROR(mAlsaControl->set("Headset Playback Volume", AUDIO_CODEC_HEADSET_GAIN), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mixer Voice", 1), error);
    CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Playback", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 PDM Switch", 1), error);
    CHECK_ERROR(mAlsaControl->set("SDT DL Volume",
                                AUDIO_ABE_SIDETONE_DL_VOL_HEADSET, -1), error);

    // Enable Capture voice path
    CHECK_ERROR(mAlsaControl->set("Analog Left Capture Route", "Headset Mic"), error);
    CHECK_ERROR(mAlsaControl->set("Analog Right Capture Route", "Headset Mic"), error);
    CHECK_ERROR(mAlsaControl->set("Capture Preamplifier Volume",
                                AUDIO_CODEC_CAPTURE_PREAMP_ATT_HEADSET, -1), error);
    CHECK_ERROR(mAlsaControl->set("Capture Volume",
                                AUDIO_CODEC_CAPTURE_VOL_HEADSET, -1), error);
    CHECK_ERROR(mAlsaControl->set("AUDUL Voice UL Volume",
                                AUDIO_ABE_AUDUL_VOICE_VOL_HEADSET, -1), error);
    CHECK_ERROR(configMicrophones(), error);
    CHECK_ERROR(configEqualizers(), error);

    return error;
}

#ifdef AUDIO_BLUETOOTH
status_t AudioModemAlsa::voiceCallCodecSetBluetooth()
{
    status_t error = NO_ERROR;

    // Enable Playback voice path
    CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mixer Voice", 1), error);
    CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Playback", 1), error);
    CHECK_ERROR(mAlsaControl->set("SDT DL Volume",
                                AUDIO_ABE_SIDETONE_DL_VOL_BLUETOOTH, -1), error);

    // Enable Capture voice path
    CHECK_ERROR(mAlsaControl->set("AUDUL Voice UL Volume",
                                AUDIO_ABE_AUDUL_VOICE_VOL_BLUETOOTH, -1), error);
    CHECK_ERROR(mAlsaControl->set("DL1 PDM Switch", 0, 0), error);
    CHECK_ERROR(mAlsaControl->set("DL1 BT_VX Switch", 1), error);
    CHECK_ERROR(configMicrophones(), error);
    CHECK_ERROR(configEqualizers(), error);

    return error;
}
#endif

status_t AudioModemAlsa::voiceCallCodecUpdateHandset()
{
    status_t error = NO_ERROR;

    switch (mPreviousAudioModemModes) {
    case AudioModemInterface::AUDIO_MODEM_HANDFREE:
        // Disable the ABE DL2 mixer used for Voice
        CHECK_ERROR(mAlsaControl->set("DL2 Mixer Voice", 0, 0), error);
        CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 0, 0), error);
        CHECK_ERROR(voiceCallCodecSetHandset(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_HEADSET:
    case AudioModemInterface::AUDIO_MODEM_BLUETOOTH:
        CHECK_ERROR(voiceCallCodecSetHandset(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_HANDSET:
    default:
        ALOGE("%s: Wrong mPreviousAudioModemModes: %04x",
                __FUNCTION__,
                mPreviousAudioModemModes);
        error = INVALID_OPERATION;
        break;

    } //switch (mPreviousAudioModemModes)

    return error;
}

status_t AudioModemAlsa::voiceCallCodecUpdateHandfree()
{
    status_t error = NO_ERROR;

    switch (mPreviousAudioModemModes) {
    case AudioModemInterface::AUDIO_MODEM_HANDSET:
    case AudioModemInterface::AUDIO_MODEM_HEADSET:
    case AudioModemInterface::AUDIO_MODEM_BLUETOOTH:
        // Disable the ABE DL1 mixer used for Voice
        CHECK_ERROR(mAlsaControl->set("DL1 Mixer Voice", 0, 0), error);
        CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 0, 0), error);
        CHECK_ERROR(voiceCallCodecSetHandfree(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_HANDFREE:
    default:
        ALOGE("%s: Wrong mPreviousAudioModemModes: %04x",
                __FUNCTION__,
                mPreviousAudioModemModes);
        error = INVALID_OPERATION;
        break;

    } //switch (mPreviousAudioModemModes)

    return error;
}

status_t AudioModemAlsa::voiceCallCodecUpdateHeadset()
{
    status_t error = NO_ERROR;

    switch (mPreviousAudioModemModes) {
    case AudioModemInterface::AUDIO_MODEM_HANDFREE:
        // Disable the ABE DL2 mixer used for Voice
        CHECK_ERROR(mAlsaControl->set("DL2 Mixer Voice", 0, 0), error);
        CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 0, 0), error);
        CHECK_ERROR(voiceCallCodecSetHeadset(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_HANDSET:
    case AudioModemInterface::AUDIO_MODEM_BLUETOOTH:
        CHECK_ERROR(voiceCallCodecSetHeadset(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_HEADSET:
    default:
        ALOGE("%s: Wrong mPreviousAudioModemModes: %04x",
                __FUNCTION__,
                mPreviousAudioModemModes);
        error = INVALID_OPERATION;
        break;

    } //switch (mPreviousAudioModemModes)

    return error;
}

#ifdef AUDIO_BLUETOOTH
status_t AudioModemAlsa::voiceCallCodecUpdateBluetooth()
{
    status_t error = NO_ERROR;

    switch (mPreviousAudioModemModes) {
    case AudioModemInterface::AUDIO_MODEM_HANDFREE:
        // Disable the ABE DL2 mixer used for Voice
        CHECK_ERROR(mAlsaControl->set("DL2 Mixer Voice", 0, 0), error);
        CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 0, 0), error);
        CHECK_ERROR(voiceCallCodecSetBluetooth(), error);
    case AudioModemInterface::AUDIO_MODEM_HANDSET:
    case AudioModemInterface::AUDIO_MODEM_HEADSET:
        CHECK_ERROR(voiceCallCodecSetBluetooth(), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_BLUETOOTH:
    default:
        ALOGE("%s: Wrong mPreviousAudioModemModes: %04x",
                __FUNCTION__,
                mPreviousAudioModemModes);
        error = INVALID_OPERATION;
        break;

    } //switch (mPreviousAudioModemModes)

    return error;
}
#endif // AUDIO_BLUETOOTH

status_t AudioModemAlsa::voiceCallCodecUpdate()
{
    status_t error = NO_ERROR;

    ALOGV("Update Audio Codec Voice call: %04x", mCurrentAudioModemModes);

    if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDSET) {
        error = voiceCallCodecPCMReset();
        error = voiceCallCodecUpdateHandset();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDFREE) {
        error = voiceCallCodecPCMReset();
        error = voiceCallCodecUpdateHandfree();
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HEADSET) {
        error = voiceCallCodecPCMReset();
        error = voiceCallCodecUpdateHeadset();
#ifdef AUDIO_BLUETOOTH
    } else if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        error = voiceCallCodecPCMReset();
        error = voiceCallCodecUpdateBluetooth();
#endif
    } else {
        ALOGE("Audio Modem mode not supported: %04x", mCurrentAudioModemModes);
        return INVALID_OPERATION;
    }

    error = voiceCallCodecPCMSet();
    error = voiceCallSidetoneSet(mCurrentAudioModemModes);

    return error;
}

status_t AudioModemAlsa::multimediaCodecUpdate()
{
    status_t error = NO_ERROR;

    CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 1), error);
    CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 1), error);
    configEqualizers();

    return error;
}

status_t AudioModemAlsa::voiceCallSidetoneSet(int mCurrentAudioModemModes)
{
    status_t error = NO_ERROR;

    CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Capture", 1), error);

    if (mCurrentAudioModemModes & AudioModemInterface::AUDIO_MODEM_HANDSET) {
        CHECK_ERROR(mAlsaControl->set("SDT UL Volume",
                                AUDIO_ABE_SIDETONE_UL_VOL_HANDSET, -1), error);
    } else if (mCurrentAudioModemModes &
                                AudioModemInterface::AUDIO_MODEM_HANDFREE) {
        CHECK_ERROR(mAlsaControl->set("SDT UL Volume",
                                AUDIO_ABE_SIDETONE_UL_VOL_HANDFREE, -1), error);
    } else if (mCurrentAudioModemModes &
                                AudioModemInterface::AUDIO_MODEM_HEADSET) {
        CHECK_ERROR(mAlsaControl->set("SDT UL Volume",
                                AUDIO_ABE_SIDETONE_UL_VOL_HEADSET, -1), error);
#ifdef AUDIO_BLUETOOTH
    } else if (mCurrentAudioModemModes &
                                AudioModemInterface::AUDIO_MODEM_BLUETOOTH) {
        CHECK_ERROR(mAlsaControl->set("SDT UL Volume",
                                AUDIO_ABE_SIDETONE_UL_VOL_BLUETOOTH, -1),
                                error);
#endif
    } else {
        ALOGE("Audio Modem mode not supported: %04x", mCurrentAudioModemModes);
        CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Capture", 0, 0), error);
        return INVALID_OPERATION;
    }
    return error;
}

status_t AudioModemAlsa::voiceCallSidetoneReset()
{
    status_t error = NO_ERROR;
    // Disable Sidetone
    CHECK_ERROR(mAlsaControl->set("SDT UL Volume", 0, -1), error);
    CHECK_ERROR(mAlsaControl->set("Sidetone Mixer Capture", 0, 0), error);

    return error;
}

status_t AudioModemAlsa::voiceCallCodecReset()
{
    status_t error = NO_ERROR;

    ALOGV("Stop Audio Codec Voice call");
    error = voiceCallSidetoneReset();

    error = voiceCallCodecPCMReset();

    error = voiceCallCodecStop();

    return error;
}

status_t AudioModemAlsa::voiceCallCodecStop()
{
    status_t error = NO_ERROR;

    // Enable Playback voice path
    CHECK_ERROR(mAlsaControl->set("DL1 Mixer Voice", 0, 0), error);
    CHECK_ERROR(mAlsaControl->set("DL2 Mixer Voice", 0, 0), error);
    CHECK_ERROR(mAlsaControl->set("DL1 Mono Mixer", 0, 0), error);
    CHECK_ERROR(mAlsaControl->set("DL2 Mono Mixer", 0, 0), error);

    // Enable Capture voice path
    CHECK_ERROR(mAlsaControl->set("Analog Left Capture Route", "Off"), error);
    CHECK_ERROR(mAlsaControl->set("Analog Right Capture Route", "Off"), error);
    CHECK_ERROR(mAlsaControl->set("Capture Preamplifier Volume", 0, -1), error);
    CHECK_ERROR(mAlsaControl->set("Capture Volume", 0, -1), error);

    CHECK_ERROR(mAlsaControl->set("AMIC_UL PDM Switch", 0, 0), error);
    CHECK_ERROR(mAlsaControl->set("MUX_VX0", "None"), error);
    CHECK_ERROR(mAlsaControl->set("MUX_VX1", "None"), error);
    CHECK_ERROR(mAlsaControl->set("AUDUL Voice UL Volume", 0, -1), error);
    CHECK_ERROR(mAlsaControl->set("Voice Capture Mixer Capture", 0, 0), error);

    return error;
}

status_t AudioModemAlsa::voiceCallModemSet()
{
    status_t error = NO_ERROR;

    ALOGV("Start Audio Modem Voice call");


    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "auto")) {
        mVoiceCallSampleRate = mModem->GetVoiceCallSampleRate();
        ALOGV("Sample rate used for this voice call: %d", mVoiceCallSampleRate);
        if ((mVoiceCallSampleRate == AudioModemInterface::INVALID_SAMPLE_RATE) ||
            (mVoiceCallSampleRate == AudioModemInterface::PCM_44_1_KHZ))  {
            ALOGE("Invalid Sample rate used for this voice call set to 8KHz");
            mVoiceCallSampleRate = AudioModemInterface::PCM_8_KHZ;
        }
    }

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        mVoiceCallSampleRate = AudioModemInterface::PCM_16_KHZ;
    } else {
        mVoiceCallSampleRate = AudioModemInterface::PCM_8_KHZ;
    }
    error = mModem->setModemRouting(mCurrentAudioModemModes,
            mVoiceCallSampleRate);
    if (error < 0) {
        ALOGE("Unable to set Modem Voice Call routing: %s", strerror(error));
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
        ALOGE("Unable to set Modem Voice Call multimic.: %s", strerror(error));
        return error;
    }

    error =  mModem->OpenModemVoiceCallStream();
    if (error < 0) {
        ALOGE("Unable to open Modem Voice Call stream: %s", strerror(error));
        return error;
    }

    return error;
}

status_t AudioModemAlsa::voiceCallModemUpdate()
{
    status_t error = NO_ERROR;

    ALOGV("Update Audio Modem Voice call");

    if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_SAMPLERATE].name,
                "16Khz")) {
        mVoiceCallSampleRate = AudioModemInterface::PCM_16_KHZ;
    } else {
        mVoiceCallSampleRate = AudioModemInterface::PCM_8_KHZ;
    }
    error = mModem->setModemRouting(mCurrentAudioModemModes,
            mVoiceCallSampleRate);
    if (error < 0) {
        ALOGE("Unable to set Modem Voice Call routing: %s", strerror(error));
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
        ALOGE("Unable to set Modem Voice Call multimic.: %s", strerror(error));
        return error;
    }

    return error;
}

status_t AudioModemAlsa::voiceCallModemReset()
{
    status_t error = NO_ERROR;

    ALOGV("Stop Audio Modem Voice call");

    error = mModem->CloseModemVoiceCallStream();
    if (error < 0) {
        ALOGE("Unable to close Modem Voice Call stream: %s", strerror(error));
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

        ALOGV("%s: in call volume level to apply: %d", voiceCallVolumeProp[i].volumeName,
                setVolume);

        error = alsaControl->set(voiceCallVolumeProp[i].volumeName, setVolume, 0);
        if (error < 0) {
            ALOGE("%s: error applying in call volume: %d", voiceCallVolumeProp[i].volumeName,
                    setVolume);
            return error;
        }
        i++;
    }
    return NO_ERROR;
}

status_t AudioModemAlsa::microphoneChosen(void)
{
    status_t error = NO_ERROR;

    // initialize mics from system property defaults
    CHECK_ERROR(propModemMgr.setFromProperty((String8)Omap4ALSAManager::MAIN_MIC,
                (const String8)"AMic0"), error);
    CHECK_ERROR(propModemMgr.setFromProperty((String8)Omap4ALSAManager::SUB_MIC,
                (const String8)"AMic1"), error);

    return error;
}

status_t AudioModemAlsa::configMicrophones(void)
{
    ALSAControl control("hw:00");
    String8 keyMain = (String8)Omap4ALSAManager::MAIN_MIC;
    String8 keySub = (String8)Omap4ALSAManager::SUB_MIC;
    String8 main;
    String8 sub;
    status_t error = NO_ERROR;
    char dmicVolumeName[20];
    char dmicMainName[7], dmicSubName[7];

    CHECK_ERROR(propModemMgr.get(keyMain, main), error);
    CHECK_ERROR(propModemMgr.get(keySub, sub), error);

    strcpy(dmicMainName ,main.string());
    strcpy(dmicSubName ,sub.string());

    switch (mCurrentAudioModemModes) {
    case AudioModemInterface::AUDIO_MODEM_HANDFREE:
        if ((strncmp(dmicMainName, "D", 1) == 0) &&
            (strncmp(dmicSubName, "D", 1) == 0)) {
            CHECK_ERROR(control.set("AMIC_UL PDM Switch", 0, 0), error);
        } else {
            CHECK_ERROR(control.set("AMIC_UL PDM Switch", 1), error);
            CHECK_ERROR(control.set("AMIC UL Volume",
                        AUDIO_ABE_AMIC_UL_VOL_HANDFREE, -1), error);
        }

        if (strncmp(dmicMainName, "D", 1) == 0) {
            sprintf(dmicVolumeName, "DMIC%c UL Volume", dmicMainName[4] + 1);
            CHECK_ERROR(control.set(dmicVolumeName,
                        AUDIO_ABE_DMIC_MAIN_UL_VOL_HANDFREE, -1), error);
        }
        if (strncmp(dmicSubName, "D", 1) == 0) {
            sprintf(dmicVolumeName, "DMIC%c UL Volume", dmicMainName[4] + 1);
            CHECK_ERROR(control.set(dmicVolumeName,
                        AUDIO_ABE_DMIC_SUB_UL_VOL_HANDFREE, -1), error);
        }

        CHECK_ERROR(control.set("MUX_VX0", dmicMainName), error);
        if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) {
            CHECK_ERROR(control.set("MUX_VX1", dmicSubName), error);
        } else {
            CHECK_ERROR(control.set("MUX_VX1", dmicMainName), error);
        }
        break;

    case AudioModemInterface::AUDIO_MODEM_HANDSET:
        if ((strncmp(dmicMainName, "D", 1) == 0) &&
            (strncmp(dmicSubName, "D", 1) == 0)) {
            CHECK_ERROR(control.set("AMIC_UL PDM Switch", 0, 0), error);
        } else {
            CHECK_ERROR(control.set("AMIC_UL PDM Switch", 1), error);
            CHECK_ERROR(control.set("AMIC UL Volume",
                        AUDIO_ABE_AMIC_UL_VOL_HANDSET, -1), error);
        }

        if (strncmp(dmicMainName, "D", 1) == 0) {
            sprintf(dmicVolumeName, "DMIC%c UL Volume", dmicMainName[4] + 1);
            CHECK_ERROR(control.set(dmicVolumeName,
                        AUDIO_ABE_DMIC_MAIN_UL_VOL_HANDSET, -1), error);
        }
        if (strncmp(dmicSubName, "D", 1) == 0) {
            sprintf(dmicVolumeName, "DMIC%c UL Volume", dmicMainName[4] + 1);
            CHECK_ERROR(control.set(dmicVolumeName,
                        AUDIO_ABE_DMIC_SUB_UL_VOL_HANDSET, -1), error);
        }

        CHECK_ERROR(control.set("MUX_VX0", dmicMainName), error);
        if (!strcmp(mDeviceProp->settingsList[AUDIO_MODEM_VOICE_CALL_MULTIMIC].name,
                    "Yes")) {
            ALOGV("dual mic. enabled");
            CHECK_ERROR(control.set("MUX_VX1", dmicSubName), error);
        } else {
            CHECK_ERROR(control.set("MUX_VX1", dmicMainName), error);
        }
        break;

    case AudioModemInterface::AUDIO_MODEM_HEADSET:
        // In headset the microphone allways comes from analog
        CHECK_ERROR(control.set("AMIC_UL PDM Switch", 1), error);
        CHECK_ERROR(mAlsaControl->set("MUX_VX0", "AMic0"), error);
        CHECK_ERROR(mAlsaControl->set("MUX_VX1", "AMic0"), error);
        break;

    case AudioModemInterface::AUDIO_MODEM_BLUETOOTH:
        CHECK_ERROR(control.set("AMIC_UL PDM Switch", 0, 0), error);
        CHECK_ERROR(mAlsaControl->set("MUX_VX0", "BT Left"), error);
        CHECK_ERROR(mAlsaControl->set("MUX_VX1", "BT Left"), error);
        CHECK_ERROR(control.set("BT UL Volume",
                    AUDIO_ABE_BT_MIC_UL_VOL, -1), error);
        break;

    default:
        ALOGE("%s: Wrong mCurrentAudioModemModes: %04x",
                __FUNCTION__,
                mCurrentAudioModemModes);
        error = INVALID_OPERATION;
        break;
    } //switch (modem_mode)

    //always enable vx mixer
    CHECK_ERROR(mAlsaControl->set("Voice Capture Mixer Capture", 1), error);

    return error;
}

status_t AudioModemAlsa::configEqualizers(void)
{
    ALSAControl control("hw:00");
    status_t error = NO_ERROR;
    String8 equalizerSetting;
    String8 keyEqualizer;

    keyEqualizer = (String8)Omap4ALSAManager::AMIC_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("AMIC Equalizer", equalizerSetting.string()), error);

    keyEqualizer = (String8)Omap4ALSAManager::DMIC_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("DMIC Equalizer", equalizerSetting.string()), error);

    keyEqualizer = (String8)Omap4ALSAManager::DL1_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("DL1 Equalizer", equalizerSetting.string()), error);

    keyEqualizer = (String8)Omap4ALSAManager::DL2L_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("DL2 Left Equalizer", equalizerSetting.string()), error);

    keyEqualizer = (String8)Omap4ALSAManager::DL2R_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("DL2 Right Equalizer", equalizerSetting.string()), error);

    keyEqualizer = (String8)Omap4ALSAManager::SDT_EQ_PROFILE;
    CHECK_ERROR(propModemMgr.get(keyEqualizer, equalizerSetting), error);
    CHECK_ERROR(control.set("Sidetone Equalizer", equalizerSetting.string()), error);

    return error;
}
} // namespace android
