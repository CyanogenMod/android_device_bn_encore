/* audio_modem_interface.h
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

#ifndef ANDROID_AUDIO_MODEM_INTERFACE_H
#define ANDROID_AUDIO_MODEM_INTERFACE_H

#include <media/AudioSystem.h>

namespace android
{

class AudioModemInterface
{
    public:

    virtual         ~AudioModemInterface() {};

    /**
    * AudioModemInterface is the abstraction interface for the audio modem.
    *
    */

    enum audio_modem_modes {
        AUDIO_MODEM_HANDSET =   AudioSystem::DEVICE_OUT_EARPIECE,
        AUDIO_MODEM_HANDFREE =  AudioSystem::DEVICE_OUT_SPEAKER,
        AUDIO_MODEM_HEADSET =   AudioSystem::DEVICE_OUT_WIRED_HEADSET |
                                AudioSystem::DEVICE_OUT_WIRED_HEADPHONE,
        AUDIO_MODEM_AUX =    AudioSystem::DEVICE_OUT_AUX_DIGITAL,
        AUDIO_MODEM_BLUETOOTH = AudioSystem::DEVICE_OUT_BLUETOOTH_SCO |
                                AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT,
        AUDIO_MODEM_ALL = AUDIO_MODEM_HANDSET | AUDIO_MODEM_HANDFREE |
                          AUDIO_MODEM_HEADSET | AUDIO_MODEM_AUX |
                          AUDIO_MODEM_BLUETOOTH,
        AUDIO_MODEM_NONE = 0
    };

    enum audio_modem_sample_rate {
        PCM_8_KHZ = 0,
        PCM_16_KHZ,
        PCM_44_1_KHZ,
        INVALID_SAMPLE_RATE
    };

    enum audio_modem_mic_setting {
        MODEM_SINGLE_MIC = 0,
        MODEM_DOUBLE_MIC,
        INVALID_MODEM_MIC_SETTING
    };

    enum audio_modem_channel_used {
        MODEM_CHANNEL_I2S1 = 0,
        MODEM_CHANNEL_VOICE,
        MODEM_CHANNEL_VOICE_SECOND_CHANNEL,
        MODEM_CHANNEL_VOICE_THIRD_CHANNEL,
        INVALID_MODEM_CHANNEL_USED_SETTING
    };

    enum audio_modem_audio_sample_type {
        MODEM_AUDIO_SAMPLE_SYSTEM_SOUND = 0,
        MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_NEAREND_VOICE,
        MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_FAREND_VOICE,
        MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_NONE,
        INVALID_MODEM_AUDIO_SAMPLE_SETTING
    };

    enum audio_modem_record_sample_type {
        MODEM_AUDIO_RECORD_VOICE_NO_ECHO_CANCEL = 0,
        MODEM_AUDIO_RECORD_VOICE_ECHO_CANCEL,
        INVALID_MODEM_AUDIO_RECORD_SETTING
    };

    /**
        * check to see if the audio modem hardware interface has been initialized.
        * return status based on values defined in include/utils/Errors.h
        */
    virtual status_t    initCheck() = 0;

    /**
    * Audio modem routing methods.
    * Audio routes can be ( AUDIO_MODEM_HANDSET | AUDIO_MODEM_HANDFREE |
    *                       AUDIO_MODEM_HEADSET | AUDIO_MODEM_CARKIT |
    *                       AUDIO_MODEM_BLUETOOTH)
    * sampleRate: indicates the PCM sample rate used. It can be PCM_8KHZ or PCM_16KHZ.
    *
    * setModemRouting sets the routes. This is called at voice call startup. It is
    * also called when a new device is connected, such as a wired headset is
    * plugged in or a Bluetooth headset is paired.
    * It can be used to change the acoustic mode of the modem.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    setModemRouting(uint32_t routes=0,
                                        uint32_t sampleRate=0) = 0;

    /**
    * Set the audio volume of the System sound mixed with voice call.
    * It can be used to select if the system sound can be mixed with far end or near end or both paths.
    *
    * nearEndVolume: set the audio volume of the System sound mixed with voice call near end path.
    *                Range is between 0.0 and 1.0.
    * farEndVolume: set the audio volume of the System sound mixed with voice call far end path.
    *               Range is between 0.0 and 1.0.
    *
    *  In general the volume is a ratio versus the Voice call volume:
    *       1.0 means System sound has the same volume as Voice call
    *       0.5 means System sound has 50% of the Voice call volume
    *       0.0 means System sound is muted
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    setModemAudioOutputVolume(float nearEndvolume=0,
                            float farEndvolume=0) = 0;

    /**
    * Set the audio volume used to record the near/far end voices.
    * It can be used to select which path can be recorded : far end or near end or both paths.
    *
    * nearEndVolume: set the audio volume of recorded voice call near end path.
    *                Range is between 0.0 and 1.0.
    * farEndVolume: set the audio volume of recorded voice call far end path.
    *               Range is between 0.0 and 1.0.
    *
    *  In general the volume is a ratio versus the Voice call volume:
    *       1.0 means System sound has the same volume as Voice call
    *       0.5 means System sound has 50% of the Voice call volume
    *       0.0 means System sound is muted
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    setModemAudioInputVolume(float nearEndvolume=0,
                            float farEndvolume=0) = 0;

    /**
    * Set the number of microphones the modem needs to enable.
    *
    * multiMic set the number of Microphones used by the modem during voice call.
    * It can be MODEM_SINGLE_MIC or MODEM_DOUBLE_MIC.
    * This can be changed during the voice call for example only 1 microphone can be used on Bluetooth.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    setModemVoiceCallMultiMic(int multiMic=0) = 0;

    /**
    * Create and open the Modem Voice call input/output stream.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    OpenModemVoiceCallStream() = 0;

    /**
    * Creates and open the Modem Audio output stream mixed with the voice call.
    *
    * sampleRate: indicates the PCM sample rate used. It can be PCM_8KHZ or PCM_16KHZ.
    * channelUsed: indicates which type of audio hardware channel is used to
    *               sent audio to the modem.
    * MODEM_CHANNEL_VOICE : Audio output stream is mixed with voice call stream.
    * MODEM_CHANNEL_VOICE_SECOND_CHANNEL: Audio output stream is sent to modem voice call
    *                                   second channel (voice channel used TDM mode
    *                                   and voice is on the other channel,
    *                                   dual mic feature (if used) will be stopped).
    * MODEM_CHANNEL_VOICE_THIRD_CHANNEL: Audio output stream is sent to modem voice call
    *                                   third channel (voice channel used TDM mode
    *                                   and voice is on the first channel,
    *                                   second microphone used the second channel).
    * MODEM_CHANNEL_I2S1  : Audio output stream is sent to the modem I2S1 port.
    *
    * audioSampleType: indicates the type of audio sample sent to the modem.
    * MODEM_AUDIO_SAMPLE_SYSTEM_SOUND: only audio system sound is sent to the modem.
    * MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_NEAREND_VOICE: audio system sound mixed
    *                                                with nearend voice is sent
    *                                               to the modem.
    * MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_FAREND_VOICE: audio system sound mixed
    *                                               with farend voice is sent
    *                                               to the modem.
    * MODEM_AUDIO_SAMPLE_SYSTEM_SOUND_NONE: no audio system sound is sent to the modem
    *                                       but a system sound is played. it's used to
    *                                       indicate to the modem that audio environment
    *                                       changed, acoustic algo can be adapted if possible.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    OpenModemAudioOutputStream(uint32_t sampleRate=0,
                            int channelUsed=0,
                            int audioSampleType=0) = 0;

    /**
    * Create and open the Modem Audio input stream.
    * This input stream consists in recording the far end and/or near end of the voice call
    *
    * sampleRate: indicates the PCM sample rate used. It can be PCM_8KHZ or PCM_16KHZ.
    * channelUsed: indicates which type of audio hardware channel is used to
    *               sent audio to the modem.
    * MODEM_CHANNEL_VOICE: Audio input stream is mixed with voice call stream
    * MODEM_CHANNEL_VOICE_SECOND_CHANNEL: Audio input stream is received from modem voice call
    *                                   second channel (voice channel used TDM mode
    *                                   and voice is on the other channel).
    * MODEM_CHANNEL_I2S1: Audio input stream is received from the modem I2S1 port
    *
    * recordSampleType: indicates the type of audio sample received from the modem.
    * MODEM_AUDIO_RECORD_VOICE_NO_ECHO_CANCEL: modem sent VOICE without any echo cancelation.
                            Echo cancelation algo can be added by the Audio HAL.
    * MODEM_AUDIO_RECORD_VOICE_ECHO_CANCEL: modem sent VOICE with echo cancelation.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    OpenModemAudioInputStream(uint32_t sampleRate=0,
                            int channelUsed=0,
                            int recordSampleType=0) = 0;

    /**
    * Close the Modem Voice call.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    CloseModemVoiceCallStream() = 0;

    /**
    * Close the Modem Audio output stream mixed with the Voice call.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    CloseModemAudioOutputStream() = 0;

    /**
    * Close the Modem Audio input stream.
    * This input stream consists in recording the far end and/or near end of the voice call.
    *
    * Return status based on values defined in include /utils/Errors.h
    */
    virtual status_t    CloseModemAudioInputStream() = 0;

    /**
    * Get the voice call sample rate used.
    *
    * Return the voice call sample rate used based on values defined in audio_modem_sample_rate.
    *
    * !!!! This method can block until the modem voice call sample rate is known
    */
    virtual uint32_t    GetVoiceCallSampleRate() = 0;
};

// ----------------------------------------------------------------------------

extern "C" AudioModemInterface* createAudioModemInterface(void);

};        // namespace android
#endif    // ANDROID_AUDIO_MODEM_INTERFACE_H
