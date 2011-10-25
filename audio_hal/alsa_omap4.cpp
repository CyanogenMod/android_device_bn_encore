/* alsa_omap4.cpp
 **
 ** Copyright 2009-2011 Texas Instruments
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

#define LOG_TAG "Omap4ALSA"
#include <utils/Log.h>

#include "AudioHardwareALSA.h"
#include <media/AudioRecord.h>
#include "alsa_omap4.h"

static bool fm_enable = false;
static bool mActive = false;

namespace android
{

static int s_device_open(const hw_module_t*, const char*, hw_device_t**);
static int s_device_close(hw_device_t*);
static status_t s_init(alsa_device_t *, ALSAHandleList &);
static status_t s_open(alsa_handle_t *, uint32_t, int, uint32_t);
static status_t s_close(alsa_handle_t *);
static status_t s_standby(alsa_handle_t *);
static status_t s_route(alsa_handle_t *, uint32_t, int);
static status_t s_voicevolume(float);
static status_t s_set(const String8&);
static status_t s_resetDefaults(alsa_handle_t *handle);


#ifdef AUDIO_MODEM_TI
    AudioModemAlsa *audioModem;
#endif

    Omap4ALSAManager propMgr;

static hw_module_methods_t s_module_methods = {
    open            : s_device_open
};

extern "C" const hw_module_t HAL_MODULE_INFO_SYM = {
    tag             : HARDWARE_MODULE_TAG,
    version_major   : 1,
    version_minor   : 0,
    id              : ALSA_HARDWARE_MODULE_ID,
    name            : "Omap4 ALSA module",
    author          : "Texas Instruments",
    methods         : &s_module_methods,
    dso             : 0,
    reserved        : {0,},
};

static int s_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    alsa_device_t *dev;
    dev = (alsa_device_t *) malloc(sizeof(*dev));
    if (!dev) return -ENOMEM;

    memset(dev, 0, sizeof(*dev));

    /* initialize the procs */
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (hw_module_t *) module;
    dev->common.close = s_device_close;
    dev->init = s_init;
    dev->open = s_open;
    dev->close = s_close;
    dev->standby = s_standby;
    dev->route = s_route;
    dev->set = s_set;
    dev->voicevolume = s_voicevolume;
    dev->resetDefaults = s_resetDefaults;

    *device = &dev->common;

    LOGD("OMAP4 ALSA module opened");

    return 0;
}

static int s_device_close(hw_device_t* device)
{
    free(device);
    return 0;
}

// ----------------------------------------------------------------------------

static const int DEFAULT_SAMPLE_RATE = ALSA_DEFAULT_SAMPLE_RATE;

static void setAlsaControls(alsa_handle_t *handle, uint32_t devices, int mode, uint32_t channels);
void configMicChoices(uint32_t);
void configEqualizer (uint32_t);
void configVoiceMemo (uint32_t);

static alsa_handle_t _defaults[] = {
/*
    Desriptions and expectations for how this module interprets
    the fields of the alsa_handle_t struct

        module      : pointer to a alsa_device_t struct
        devices     : mapping Android devices to OMAP4 Front end devices
        curDev      : current Android device used by this handle
        curMode     : current Android mode used by this handle
        curChannels : current Android audio channels used by this handle
        handle      : pointer to a snd_pcm_t ALSA handle
        format      : bit, endianess according to ALSA definitions
        channels    : Integer number of channels
        sampleRate  : Desired sample rate in Hz
        latency     : Desired Delay in usec for the ALSA buffer
        bufferSize  : Desired Number of samples for the ALSA buffer
        mmap        : true (1) to use mmap, false (0) to use standard writei
        modPrivate  : pointer to the function specific to this handle
*/
    {
        module      : 0,
        devices     : OMAP4_OUT_SCO,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 2,
        sampleRate  : DEFAULT_SAMPLE_RATE,
        latency     : 200000,
        bufferSize  : DEFAULT_SAMPLE_RATE / 5,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_OUT_FM,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 2,
        sampleRate  : DEFAULT_SAMPLE_RATE,
        latency     : 200000,
        bufferSize  : DEFAULT_SAMPLE_RATE / 5,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_OUT_HDMI,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 2,
        sampleRate  : DEFAULT_SAMPLE_RATE,
        latency     : 200000,
        bufferSize  : DEFAULT_SAMPLE_RATE / 5,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_OUT_DEFAULT,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 2,
        sampleRate  : DEFAULT_SAMPLE_RATE,
        latency     : 85333,
        bufferSize  : 4096,
        mmap        : 1,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        /* not currently used */
        module      : 0,
        devices     : OMAP4_OUT_LP,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 2,
        sampleRate  : DEFAULT_SAMPLE_RATE,
        latency     : 140000,
        bufferSize  : 6144,
        mmap        : 1,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_IN_SCO,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 1,
        sampleRate  : AudioRecord::DEFAULT_SAMPLE_RATE,
        latency     : 250000,
        bufferSize  : 2048,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_IN_DEFAULT,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S16_LE,
        channels    : 1,
        sampleRate  : AudioRecord::DEFAULT_SAMPLE_RATE,
        latency     : 250000,
        bufferSize  : 2048,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },
    {
        module      : 0,
        devices     : OMAP4_IN_FM,
        curDev      : 0,
        curMode     : 0,
        curChannels : 0,
        handle      : 0,
        format      : SND_PCM_FORMAT_S32_LE,
        channels    : 2,
        sampleRate  : 48000,
        latency     : -1, // Doesn't matter, since it is buffer-less
        bufferSize  : 2048,
        mmap        : 0,
        modPrivate  : (void *)&setAlsaControls,
    },

};

// ----------------------------------------------------------------------------

const char *deviceName(alsa_handle_t *handle, uint32_t device, int mode)
{
    char pwr[PROPERTY_VALUE_MAX];
    int status = 0;
    status = property_get("omap.audio.power", pwr, "FIFO");

    if (device & OMAP4_OUT_SCO || device & OMAP4_IN_SCO)
        return BLUETOOTH_SCO_DEVICE;

    if (device & OMAP4_OUT_FM)
        return FM_TRANSMIT_DEVICE;

    if (device & OMAP4_OUT_HDMI)
        return HDMI_DEVICE;

    if (device & OMAP4_IN_FM)
        return FM_CAPTURE_DEVICE;

    if (device & OMAP4_IN_DEFAULT)
        return MM_DEFAULT_DEVICE;

    // Hostless loopback is not supported in kernel for FM with  0,6
    // so for FM Rx, the default playback device retured is MM_DEFAULT_DEVICE i.e. 0,0
    if (fm_enable)
        return MM_DEFAULT_DEVICE;

    // now that low-power is flexible in buffer size and sample rate
    // a system property can be used to toggle
    if ((device & OMAP4_OUT_LP) ||
       (strcmp(pwr, "PingPong") == 0))
        return MM_LP_DEVICE;

    return MM_DEFAULT_DEVICE;
}

snd_pcm_stream_t direction(alsa_handle_t *handle)
{
    return (handle->devices & AudioSystem::DEVICE_OUT_ALL) ? SND_PCM_STREAM_PLAYBACK
            : SND_PCM_STREAM_CAPTURE;
}

const char *streamName(alsa_handle_t *handle)
{
    return snd_pcm_stream_name(direction(handle));
}

status_t setHardwareParams(alsa_handle_t *handle)
{
    snd_pcm_hw_params_t *hardwareParams;
    status_t err;

    snd_pcm_uframes_t periodSize = 0, bufferSize = 0, reqBuffSize = 0;
    unsigned int periodTime, bufferTime;
    unsigned int requestedRate = handle->sampleRate;
    int numPeriods = 0;
    char pwr[PROPERTY_VALUE_MAX];
    int status = 0;

    // snd_pcm_format_description() and snd_pcm_format_name() do not perform
    // proper bounds checking.
    bool validFormat = (static_cast<int> (handle->format)
            > SND_PCM_FORMAT_UNKNOWN) && (static_cast<int> (handle->format)
            <= SND_PCM_FORMAT_LAST);
    const char *formatDesc = validFormat ? snd_pcm_format_description(
            handle->format) : "Invalid Format";
    const char *formatName = validFormat ? snd_pcm_format_name(handle->format)
            : "UNKNOWN";

    // device name will only return LP device hw06 if the property is set
    // or if the system is explicitly opening and routing to OMAP4_OUT_LP
    const char* device = deviceName(handle,
                                    handle->curDev,
                                    AudioSystem::MODE_NORMAL);

    if (snd_pcm_hw_params_malloc(&hardwareParams) < 0) {
        LOG_ALWAYS_FATAL("Failed to allocate ALSA hardware parameters!");
        return NO_INIT;
    }

    err = snd_pcm_hw_params_any(handle->handle, hardwareParams);
    if (err < 0) {
        LOGE("Unable to configure hardware: %s", snd_strerror(err));
        goto done;
    }

    // Set the interleaved read and write format.
    if (handle->mmap) {
        snd_pcm_access_mask_t *mask =
            (snd_pcm_access_mask_t *)alloca(snd_pcm_access_mask_sizeof());
        snd_pcm_access_mask_none(mask);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        err = snd_pcm_hw_params_set_access_mask(
                        handle->handle,
                        hardwareParams,
                        mask);
    } else  {
        err = snd_pcm_hw_params_set_access(
                        handle->handle,
                        hardwareParams,
                        SND_PCM_ACCESS_RW_INTERLEAVED);
    }
    if (err < 0) {
        LOGE("Unable to configure PCM read/write format: %s",
                snd_strerror(err));
        goto done;
    }

    err = snd_pcm_hw_params_set_format(handle->handle, hardwareParams,
            handle->format);
    if (err < 0) {
        LOGE("Unable to configure PCM format %s (%s): %s",
                formatName, formatDesc, snd_strerror(err));
        goto done;
    }

    LOGV("Set %s PCM format to %s (%s)", streamName(handle), formatName, formatDesc);

    err = snd_pcm_hw_params_set_channels(handle->handle, hardwareParams,
            handle->channels);
    if (err < 0) {
        LOGE("Unable to set channel count to %i: %s",
                handle->channels, snd_strerror(err));
        goto done;
    }

    LOGV("Using %i %s for %s.", handle->channels,
            handle->channels == 1 ? "channel" : "channels", streamName(handle));

    err = snd_pcm_hw_params_set_rate_near(handle->handle, hardwareParams,
            &requestedRate, 0);
    if (err < 0)
        LOGE("Unable to set %s sample rate to %u: %s",
                streamName(handle), handle->sampleRate, snd_strerror(err));
    else if (requestedRate != handle->sampleRate)
        // Some devices have a fixed sample rate, and can not be changed.
        // This may cause resampling problems; i.e. PCM playback will be too
        // slow or fast.
        LOGW("Requested rate (%u HZ) does not match actual rate (%u HZ)",
                handle->sampleRate, requestedRate);
    else
        LOGV("Set %s sample rate to %u HZ", streamName(handle), requestedRate);

    if (strcmp(device, MM_LP_DEVICE) == 0) {
        numPeriods = 2;
        LOGI("Using ping-pong!");
    } else {
        numPeriods = 4;
        LOGI("Using FIFO");
    }

    //get the default array index
    for (size_t i = 0; i < ARRAY_SIZE(_defaults); i++) {
        if (_defaults[i].devices == handle->devices) {
            reqBuffSize = _defaults[i].bufferSize;
            break;
        }
    }
    bufferSize = reqBuffSize;

    // try the requested buffer size
    err = snd_pcm_hw_params_set_buffer_size_near(handle->handle, hardwareParams,
            &bufferSize);
    if (err < 0) {
        LOGE("Unable to set buffer size to %d:  %s",
                (int)bufferSize, snd_strerror(err));
        goto done;
    }
    // did we get what we asked for? we should.
    if (bufferSize != reqBuffSize) {
         LOGW("Requested buffer size %d not granted, got %d",
                (int)reqBuffSize, (int)bufferSize);
    }
    // set the period size for our buffer
    periodSize = bufferSize / numPeriods;
    err = snd_pcm_hw_params_set_period_size_near(handle->handle,
            hardwareParams, &periodSize, NULL);
    if (err < 0) {
        LOGE("Unable to set period size to %d:  %s", (int)periodSize, snd_strerror(err));
        goto done;
    }
    // check our period time
    err = snd_pcm_hw_params_get_period_time(hardwareParams, &periodTime, NULL);
    if (err < 0) {
        LOGE("Unable to get period time:  %s", snd_strerror(err));
        goto done;
    }
    // get the buffer time
    err = snd_pcm_hw_params_get_buffer_time(hardwareParams, &bufferTime, NULL);
    if (err < 0) {
        LOGE("Unable to set buffer time:  %s", snd_strerror(err));
        goto done;
    }
    // get the buffer size again in case setting the period size changed it
    err = snd_pcm_hw_params_get_buffer_size(hardwareParams, &bufferSize);
    if (err < 0) {
        LOGE("Unable to set buffer size:  %s", snd_strerror(err));
        goto done;
    }

    if (strcmp(device, MM_LP_DEVICE) == 0) {
        // we have to overwrite our ALSA size if we want to
        // force Flinger to write for each period. This is done
        // here internally becuase the latency() API is defined as const
        handle->bufferSize = periodSize;
        handle->latency = periodTime;
    } else {
        handle->bufferSize = bufferSize;
        handle->latency = bufferTime;
    }

    LOGI("Buffer size: %d", (int)(handle->bufferSize));
    LOGI("Latency: %d", (int)(handle->latency));

    // Commit the hardware parameters back to the device.
    err = snd_pcm_hw_params(handle->handle, hardwareParams);
    if (err < 0) LOGE("Unable to set hardware parameters: %s", snd_strerror(err));

    done:
    snd_pcm_hw_params_free(hardwareParams);

    return err;
}

status_t setSoftwareParams(alsa_handle_t *handle)
{
    snd_pcm_sw_params_t * softwareParams;
    int err;

    snd_pcm_uframes_t bufferSize = 0;
    snd_pcm_uframes_t periodSize = 0;
    snd_pcm_uframes_t startThreshold, stopThreshold;

    if (snd_pcm_sw_params_malloc(&softwareParams) < 0) {
        LOG_ALWAYS_FATAL("Failed to allocate ALSA software parameters!");
        return NO_INIT;
    }

    // Get the current software parameters
    err = snd_pcm_sw_params_current(handle->handle, softwareParams);
    if (err < 0) {
        LOGE("Unable to get software parameters: %s", snd_strerror(err));
        goto done;
    }

    // Configure ALSA to start the transfer when the buffer is almost full.
    snd_pcm_get_params(handle->handle, &bufferSize, &periodSize);

    if (handle->devices & AudioSystem::DEVICE_OUT_ALL ||
        handle->devices & AudioSystem::DEVICE_OUT_LOW_POWER) {
        // For playback, configure ALSA to start the transfer at period full
        startThreshold = periodSize;
        stopThreshold = bufferSize;
    } else {
        // For recording, configure ALSA to start the transfer on the
        // first frame.
        startThreshold = 1;
        if (handle->devices & OMAP4_IN_FM) {
            LOGV("Stop Threshold for FM Rx is -1");
            stopThreshold = -1; // For FM Rx via ABE
        } else
            stopThreshold = bufferSize;
    }

    err = snd_pcm_sw_params_set_start_threshold(handle->handle, softwareParams,
            startThreshold);
    if (err < 0) {
        LOGE("Unable to set start threshold to %lu frames: %s",
                startThreshold, snd_strerror(err));
        goto done;
    }

    err = snd_pcm_sw_params_set_stop_threshold(handle->handle, softwareParams,
            stopThreshold);
    if (err < 0) {
        LOGE("Unable to set stop threshold to %lu frames: %s",
                stopThreshold, snd_strerror(err));
        goto done;
    }

    // Allow the transfer to start when at least periodSize samples can be
    // processed.
    err = snd_pcm_sw_params_set_avail_min(handle->handle, softwareParams,
            periodSize);
    if (err < 0) {
        LOGE("Unable to configure available minimum to %lu: %s",
                periodSize, snd_strerror(err));
        goto done;
    }

    // Commit the software parameters back to the device.
    err = snd_pcm_sw_params(handle->handle, softwareParams);
    if (err < 0) LOGE("Unable to configure software parameters: %s",
            snd_strerror(err));

    done:
    snd_pcm_sw_params_free(softwareParams);

    return err;
}


void setAlsaControls(alsa_handle_t *handle, uint32_t devices, int mode, uint32_t channels)
{
    LOGV("%s: devices %08x mode %d channels %08x", __FUNCTION__, devices, mode, channels);
    ALSAControl control("hw:00");

    /* check whether the devices is input or not */
    /* for output devices */
    if (devices & 0x0000FFFF){
        if (devices & AudioSystem::DEVICE_OUT_SPEAKER) {
            /* OMAP4 ABE */
            control.set("DL2 Mixer Multimedia", 1);		// MM_DL    -> DL2 Mixer
            control.set("DL2 Media Playback Volume", 118);
            /* TWL6040 */
            control.set("HF Left Playback", "HF DAC");		// HFDAC L -> HF Mux
            control.set("HF Right Playback", "HF DAC");		// HFDAC R -> HF Mux
            control.set("Handsfree Playback Volume", 23);
            if (fm_enable) {
                LOGE("FM Enabled, DL2 Capture-Playback Vol ON");
                control.set("DL2 Capture Playback Volume", 115);
                control.set("DL1 Capture Playback Volume", 0, -1);
            }
            else {
                LOGI("FM Disabled, DL2 Capture-Playback Vol OFF");
                control.set("DL2 Capture Playback Volume", 0, -1);
            }
            if (propMgr.setFromProperty((String8)Omap4ALSAManager::DL2_SPEAK_MONO_MIXER, (String8)"0") == NO_ERROR) {
                String8 value;
                if (propMgr.get((String8)Omap4ALSAManager::DL2_SPEAK_MONO_MIXER, value) == NO_ERROR) {
                    LOGD("DL2 Mono Mixer value %s",value.string());
                    control.set("DL2 Mono Mixer", atoi(value.string()));
                }
            }
        } else {
            /* OMAP4 ABE */
            control.set("DL2 Mixer Multimedia", 0, 0);
            control.set("DL2 Media Playback Volume", 0, -1);
            control.set("DL2 Capture Playback Volume", 0, -1);
            /* TWL6040 */
            control.set("HF Left Playback", "Off");
            control.set("HF Right Playback", "Off");
            control.set("Handsfree Playback Volume", 0, -1);
        }

        if ((devices & AudioSystem::DEVICE_OUT_WIRED_HEADSET) ||
            (devices & AudioSystem::DEVICE_OUT_LOW_POWER)) {
            /* TWL6040 */
            control.set("HS Left Playback", "HS DAC");		// HSDAC L -> HS Mux
            control.set("HS Right Playback", "HS DAC");		// HSDAC R -> HS Mux
            control.set("Headset Playback Volume", 15);
            if (propMgr.setFromProperty((String8)Omap4ALSAManager::DL1_HEAD_MONO_MIXER, (String8)"0") == NO_ERROR) {
                String8 value;
                if (propMgr.get((String8)Omap4ALSAManager::DL1_HEAD_MONO_MIXER, value) == NO_ERROR) {
                    LOGD("DL1 Mono Mixer value %s",value.string());
                    control.set("DL1 Mono Mixer", atoi(value.string()));
                }
            }
        } else {
            /* TWL6040 */
            control.set("HS Left Playback", "Off");
            control.set("HS Right Playback", "Off");
            control.set("Headset Playback Volume", 0, -1);
        }

        if (devices & AudioSystem::DEVICE_OUT_EARPIECE) {
            /* TWL6040 */
            control.set("EP Playback", "On");		// HSDACL -> Earpiece
            control.set("Earphone Playback Volume", 15);
            if (propMgr.setFromProperty((String8)Omap4ALSAManager::DL1_EAR_MONO_MIXER, (String8)"1") == NO_ERROR) {
                String8 value;
                if (propMgr.get((String8)Omap4ALSAManager::DL1_EAR_MONO_MIXER, value) == NO_ERROR) {
                    LOGD("DL1 Mono Mixer value %s",value.string());
                    control.set("DL1 Mono Mixer", atoi(value.string()));
                }
            }
        } else {
            /* TWL6040 */
            control.set("Earphone Playback Volume", 0, -1);
            control.set("EP Playback", "Off");
        }
        if ((devices & AudioSystem::DEVICE_OUT_EARPIECE) ||
            (devices & AudioSystem::DEVICE_OUT_WIRED_HEADSET) ||
            (devices & AudioSystem::DEVICE_OUT_LOW_POWER)) {
            /* OMAP4 ABE */
            control.set("DL1 Mixer Multimedia", 1);		// MM_DL    -> DL1 Mixer
            control.set("Sidetone Mixer Playback", 1);		// DL1 Mixer-> Sidetone Mixer
            control.set("SDT DL Volume", 118);
            control.set("DL1 Media Playback Volume", 118);
            control.set("DL1 PDM Switch", 1);
            if (fm_enable) {
                LOGI("FM Enabled, DL1 Capture-Playback Vol ON");
                control.set("DL1 Capture Playback Volume", 115);
                control.set("DL2 Capture Playback Volume", 0, -1);
            }
            else {
                LOGI("FM Disabled, DL1 Capture-Playback Vol OFF");
                control.set("DL1 Capture Playback Volume", 0, -1);
            }
        } else {
            /* OMAP4 ABE */
            control.set("DL1 Mixer Multimedia", 0, 0);
            control.set("Sidetone Mixer Playback", 0, 0);
            control.set("SDT DL Volume", 0, 0);
            control.set("DL1 PDM Switch", 0, 0);
            control.set("DL1 Media Playback Volume", 0, -1);
            control.set("DL1 Capture Playback Volume", 0, -1);
        }
        if (devices & AudioSystem::DEVICE_OUT_FM_TRANSMIT) {
            /* OMAP4 ABE */
            control.set("DL1 Mixer Multimedia", 1);             // MM_DL    -> DL1 Mixer
            control.set("Sidetone Mixer Playback", 1);          // DL1 Mixer-> Sidetone Mixer
            control.set("SDT DL Volume", 118);
            control.set("DL1 Media Playback Volume", 118);
            control.set("DL1 MM_EXT Switch", 1);
            control.set("DL1 PDM Switch", 0, 0);
        } else {
            /* Disable MM_EXT Switch */
            control.set("DL1 MM_EXT Switch", 0, 0);
        }
        if ((devices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) ||
            (devices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET) ||
            (devices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT)) {
            /* OMAP4 ABE */
            /* Bluetooth: DL1 Mixer */
            control.set("DL1 Mixer Multimedia", 1);        // MM_DL    -> DL1 Mixer
            control.set("Sidetone Mixer Playback", 1);     // DL1 Mixer-> Sidetone Mixer
            control.set("SDT DL Volume", 118);
            control.set("DL1 BT_VX Switch", 1);            // Sidetone Mixer -> BT-VX-DL
            control.set("DL1 Media Playback Volume", 118);
        } else {
            control.set("DL1 BT_VX Switch", 0, 0);
        }
        if ((devices & AudioSystem::DEVICE_OUT_SPEAKER) ||
            (devices & AudioSystem::DEVICE_OUT_AUX_DIGITAL)) {
            // Setting DL2 EQ's to 800Hz cut-off frequency, as setting
            // to flat response saturates the audio quality in the
            // handsfree speakers
            control.set("DL2 Left Equalizer", "High-pass 0dB");
            control.set("DL2 Right Equalizer", "High-pass 0dB");
        }
        if ((devices & AudioSystem::DEVICE_OUT_WIRED_HEADSET) ||
            (devices & AudioSystem::DEVICE_OUT_EARPIECE)) {
            control.set("DL1 Equalizer", "Flat response");
        }
        control.set("TWL6040 Power Mode", "Low-Power");

    }

    /* for input devices */
    if (devices >> 16) {
        if (devices & AudioSystem::DEVICE_IN_BUILTIN_MIC) {
            configMicChoices(devices);
            /* TWL6040 */
            control.set("Analog Left Capture Route", "Main Mic");	// Main Mic -> Mic Mux
            control.set("Analog Right Capture Route", "Sub Mic");	// Sub Mic  -> Mic Mux
            control.set("Capture Preamplifier Volume", 1);
            control.set("Capture Volume", 4);
        } else if (devices & AudioSystem::DEVICE_IN_WIRED_HEADSET) {
            /* TWL6040 */
            control.set("Analog Left Capture Route", "Headset Mic");	// Headset Mic -> Mic Mux
            control.set("Analog Right Capture Route", "Headset Mic");	// Headset Mic -> Mic Mux
            control.set("Capture Preamplifier Volume", 1);
            control.set("Capture Volume", 4);
            control.set("AMIC_UL PDM Switch", 1);
            control.set("MUX_UL00", "AMic1");
            control.set("MUX_UL11", "AMic0");
        } else if (devices & OMAP4_IN_FM) {
            /* TWL6040 */
            control.set("Analog Left Capture Route", "Aux/FM Left");     // FM -> Mic Mux
            control.set("Analog Right Capture Route", "Aux/FM Right");   // FM -> Mic Mux
            control.set("Capture Preamplifier Volume", 1);
            control.set("Capture Volume", 1);
            control.set("AMIC_UL PDM Switch", 1);
            control.set("MUX_UL10", "AMic1");
            control.set("MUX_UL11", "AMic0");
        } else if(devices & OMAP4_IN_SCO) {
            LOGI("OMAP4 ABE set for BT SCO Headset");
            control.set("AMIC_UL PDM Switch", 0, 0);
            control.set("MUX_UL00", "BT Right");
            control.set("MUX_UL01", "BT Left");
            control.set("MUX_UL10", "BT Right");
            control.set("MUX_UL11", "BT Left");
            control.set("BT UL Volume", 120);
            control.set("Voice Capture Mixer Capture", 1);
        } else if (devices & AudioSystem::DEVICE_IN_VOICE_CALL) {
            LOGI("OMAP4 ABE set for VXREC");
            configVoiceMemo (channels);
            control.set("MUX_UL00", "VX Right");
            control.set("MUX_UL01", "VX Left");
        } else {
            /* TWL6040 */
            control.set("Analog Left Capture Route", "Off");
            control.set("Analog Right Capture Route", "Off");
            control.set("Capture Preamplifier Volume", 0, -1);
            control.set("Capture Volume", 0, -1);
            control.set("BT UL VOlume", 0, -1);        // BT UL --> MUTE
            control.set("Voice Capture Mixer Capture", 0, 0);
            control.set("AMIC_UL PDM Switch", 0, 0);
            /* ABE */
            control.set("MUX_UL00", "None");
            control.set("MUX_UL01", "None");
            control.set("MUX_UL10", "None");
            control.set("MUX_UL11", "None");
        }
    }

    handle->curDev = devices;
    handle->curMode = mode;
    handle->curChannels = channels;
}

// ----------------------------------------------------------------------------

static status_t s_init(alsa_device_t *module, ALSAHandleList &list)
{
    LOGD("Initializing devices for OMAP4 ALSA module");
    status_t status = NO_ERROR;
    list.clear();

    for (size_t i = 0; i < ARRAY_SIZE(_defaults); i++) {

        snd_pcm_uframes_t bufferSize = _defaults[i].bufferSize;

        for (size_t b = 1; (bufferSize & ~b) != 0; b <<= 1)
            bufferSize &= ~b;

        _defaults[i].module = module;
        _defaults[i].bufferSize = bufferSize;

        list.push_back(_defaults[i]);
    }

#ifdef AUDIO_MODEM_TI
    audioModem = new AudioModemAlsa();
#endif

    propMgr = Omap4ALSAManager();

    // initialize mics and power mode from system property defaults
    status = propMgr.setFromProperty((String8)Omap4ALSAManager::MAIN_MIC);
    status = propMgr.setFromProperty((String8)Omap4ALSAManager::SUB_MIC);
    status = propMgr.setFromProperty((String8)Omap4ALSAManager::POWER_MODE);

    // initialize other tunable parameters with internal default values
    status = propMgr.set((String8)Omap4ALSAManager::DL2L_EQ_PROFILE,
                         (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    status = propMgr.set((String8)Omap4ALSAManager::DL2R_EQ_PROFILE,
                         (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    status = propMgr.set((String8)Omap4ALSAManager::DL1_EQ_PROFILE,
                         (String8)Omap4ALSAManager::EqualizerProfileList[0]);
    status = propMgr.set((String8)Omap4ALSAManager::AMIC_EQ_PROFILE,
                         (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    status = propMgr.set((String8)Omap4ALSAManager::DMIC_EQ_PROFILE,
                         (String8)Omap4ALSAManager::EqualizerProfileList[1]);
    status = propMgr.set((String8)Omap4ALSAManager::DL1_EAR_MONO_MIXER,
                         (String8)"1");
    status = propMgr.set((String8)Omap4ALSAManager::DL1_HEAD_MONO_MIXER,
                         (String8)"0");
    status = propMgr.set((String8)Omap4ALSAManager::DL2_SPEAK_MONO_MIXER,
                         (String8)"0");
    status = propMgr.set((String8)Omap4ALSAManager::DL2_AUX_MONO_MIXER,
                         (String8)"0");

    // initialize voice memo gains: multimedia and tone are not recorded by default
    status = propMgr.set((String8)Omap4ALSAManager::VOICEMEMO_VUL_GAIN,
                         (String8)"0");
    status = propMgr.set((String8)Omap4ALSAManager::VOICEMEMO_VDL_GAIN,
                         (String8)"0");
    status = propMgr.set((String8)Omap4ALSAManager::VOICEMEMO_MM_GAIN,
                         (String8)"-120");
    status = propMgr.set((String8)Omap4ALSAManager::VOICEMEMO_TONE_GAIN,
                         (String8)"-120");

    return NO_ERROR;
}

static status_t s_open(alsa_handle_t *handle, uint32_t devices, int mode, uint32_t channels)
{
    // Close off previously opened device.
    // It would be nice to determine if the underlying device actually
    // changes, but we might be recovering from an error or manipulating
    // mixer settings (see asound.conf).
    //
    s_close(handle);

    LOGD("open called for devices %08x in mode %d channels %08x...", devices, mode, channels);

    const char *stream = streamName(handle);
    const char *devName = deviceName(handle, devices, mode);

#ifdef AUDIO_MODEM_TI
    audioModem->voiceCallControlsMutexLock();
#endif

    // ASoC multicomponent requires a valid path (frontend/backend) for
    // the device to be opened
    setAlsaControls(handle, devices, mode, channels);

#ifdef AUDIO_MODEM_TI
    audioModem->voiceCallControlsMutexUnlock();
    audioModem->voiceCallControls(devices, mode, true);
#endif

    // The PCM stream is opened in blocking mode, per ALSA defaults.  The
    // AudioFlinger seems to assume blocking mode too, so asynchronous mode
    // should not be used.
    int err = snd_pcm_open(&handle->handle, devName, direction(handle), 0);

    if (err < 0) {
        LOGE("Failed to initialize ALSA %s device '%s': %s", stream, devName, strerror(err));
        return NO_INIT;
    }
    LOGV("snd_pcm_open(%p, %s, %s, 0)", handle->handle, devName,
         (direction(handle) == SND_PCM_STREAM_PLAYBACK) ? "SND_PCM_STREAM_PLAYBACK" : "SND_PCM_STREAM_CAPTURE");

    mActive = true;

    err = setHardwareParams(handle);

    if (err == NO_ERROR) err = setSoftwareParams(handle);

    LOGI("Initialized ALSA %s device '%s'", stream, devName);

    if (fm_enable) {
        LOGI("Triggering McPDM DL");
        snd_pcm_start(handle->handle);
    }

    // For FM Rx through ABE, McPDM UL needs to be triggered
    if (devices &  OMAP4_IN_FM) {
       LOGI("Triggering McPDM UL");
       fm_enable = true;
       snd_pcm_start(handle->handle);
    }
    return err;
}

static status_t s_close(alsa_handle_t *handle)
{
    status_t err = NO_ERROR;
    snd_pcm_t *h = handle->handle;
    handle->handle = 0;
    handle->curDev = 0;
    handle->curMode = 0;
    handle->curChannels = 0;
    if (h) {
        snd_pcm_drain(h);
        err = snd_pcm_close(h);
        LOGV("snd_pcm_close(%p): %s(%d) ", h,
             err != 0 ? strerror(err) : "no error",
             err != 0 ? err : 0);
        mActive = false;
    }

    return err;
}

/*
    this is same as s_close, but don't discard
    the device/mode info. This way we can still
    close the device, hit idle and power-save, reopen the pcm
    for the same device/mode after resuming
*/
static status_t s_standby(alsa_handle_t *handle)
{
    status_t err = NO_ERROR;
    snd_pcm_t *h = handle->handle;
    handle->handle = 0;
    LOGV("In omap4 standby\n");
    if (h) {
        snd_pcm_drain(h);
        err = snd_pcm_close(h);
        LOGV("snd_pcm_close(%p): %s(%d) ", h,
             err != 0 ? strerror(err) : "no error",
             err != 0 ? err : 0);
        mActive = false;
        LOGE("called drain&close\n");
    }

    return err;
}

static status_t s_route(alsa_handle_t *handle, uint32_t devices, int mode)
{
    status_t status = NO_ERROR;

    LOGD("route called for devices %08x in mode %d...", devices, mode);

    if (!devices) {
        fm_enable = false;
        LOGV("Ignore the audio routing change as there's no device specified");
        return NO_ERROR;
    }

    if (handle->curDev != devices) {
        if (mActive) {
            status = s_open(handle, devices, mode, handle->curChannels);
        } else {
#ifdef AUDIO_MODEM_TI
            audioModem->voiceCallControlsMutexLock();
#endif
            setAlsaControls(handle, devices, mode, handle->curChannels);
#ifdef AUDIO_MODEM_TI
            audioModem->voiceCallControlsMutexUnlock();
#endif
        }
    }else if (fm_enable) {
        /* FM Rx requires re-opening of playback path
         * for FM Rx mixer settings to come into effect
         * else FM Rx audio is not heard
         */
         status = s_open(handle, devices, mode, handle->curChannels);
    }

#ifdef AUDIO_MODEM_TI
        status = audioModem->voiceCallControls(devices, mode, false);
#endif

    return status;
}

static status_t s_voicevolume(float volume)
{
    status_t status = NO_ERROR;

#ifdef AUDIO_MODEM_TI
        ALSAControl control("hw:00");
        if (audioModem) {
            status = audioModem->voiceCallVolume(&control, volume);
        } else {
            LOGE("Audio Modem not initialized: voice volume can't be applied");
            status = NO_INIT;
        }
#endif

    return status;
}

static status_t s_set(const String8& keyValuePairs)
{
    AudioParameter p = AudioParameter(keyValuePairs);
    String8 key = (String8) Omap4ALSAManager::MAIN_MIC;
    String8 value;
    unsigned int i = 0;

    LOGI("set:: %s", keyValuePairs.string());
    while (i < propMgr.size()) {
        if (p.get(propMgr.mParams.keyAt(i), value) == NO_ERROR) {
            if(propMgr.set(propMgr.mParams.keyAt(i), value) == BAD_VALUE) {
                LOGE("PropMgr.set failed to validate new value for %s=%s",
                      propMgr.mParams.keyAt(i).string(), value.string());
            }
            else {
                LOGV("PropMgr.set %s::%s", propMgr.mParams.keyAt(i).string(), value.string());
                // @TODO: update any controls that should
                // based on which property was KVP was sent
                p.remove(key);
            }
            // we found it, so...
            break;
        }
        i++;
    }
    if (p.size()) {
        return BAD_VALUE;
    } else {
        return NO_ERROR;
    }
}

void configMicChoices (uint32_t devices) {

    ALSAControl control("hw:00");
    String8 keyMain = (String8)Omap4ALSAManager::MAIN_MIC;
    String8 keySub = (String8)Omap4ALSAManager::SUB_MIC;
    String8 main;
    String8 sub;

    if(propMgr.get(keyMain, main) == NO_ERROR)
        control.set("MUX_UL00", main.string());

    if(propMgr.get(keySub, sub) == NO_ERROR)
        control.set("MUX_UL01", sub.string());

    // if either mic is analog, turn on AMIC_UL_PDM switch
    if(strncmp(main.string(), "A", 1) == 0 ||
        strncmp(sub.string() , "A", 1) == 0) {
        control.set("AMIC_UL PDM Switch", 1);
    } else {
        control.set("AMIC_UL PDM Switch", 0, 0);
    }
    // if mic is digital, turn up the associated gain
    if(strncmp(main.string(), "DMic0", 5) == 0 ||
        strncmp(sub.string() , "DMic0", 5) == 0) {
        control.set("DMIC1 UL Volume", 120);      // DMIC1: 1dB=Mute --> 149dB
    } else if(strncmp(main.string(), "DMic1", 5) == 0 ||
        strncmp(sub.string() , "DMic1", 5) == 0) {
        control.set("DMIC2 UL Volume", 120);      // DMIC2: 1dB=Mute --> 149dB
    } else if(strncmp(main.string(), "DMic2", 5) == 0 ||
        strncmp(sub.string() , "DMic2", 5) == 0) {
        control.set("DMIC3 UL Volume", 120);      // DMIC3: 1dB=Mute --> 149dB
    } else {
        control.set("DMIC1 UL Volume", 1);        // DMIC1 -> MUTE
        control.set("DMIC2 UL Volume", 1);        // DMIC1 -> MUTE
        control.set("DMIC3 UL Volume", 1);        // DMIC1 -> MUTE
    }
    LOGI("main mic selected %s", main.string());
    LOGI("sub mic selected %s", sub.string());

}

void configEqualizer (uint32_t devices) {

    ALSAControl control("hw:00");

    if ((devices & AudioSystem::DEVICE_IN_BUILTIN_MIC) ||
        (devices & AudioSystem::DEVICE_IN_BACK_MIC)) {
        control.set("DMIC Equalizer", "Flat response");
    }

    if ((devices & AudioSystem::DEVICE_IN_BUILTIN_MIC) ||
        (devices & AudioSystem::DEVICE_IN_BACK_MIC) ||
        (devices & AudioSystem::DEVICE_IN_WIRED_HEADSET) ||
        (devices & AudioSystem::DEVICE_IN_AUX_DIGITAL) ||
        (devices & AudioSystem::DEVICE_IN_FM_ANALOG)) {
        control.set("AMIC Equalizer", "Flat response");
    }
}
static status_t s_resetDefaults(alsa_handle_t *handle)
{
    return setHardwareParams(handle);
}

void configVoiceMemo (uint32_t channels) {

    ALSAControl control("hw:00");
    int voiceUlGain = -120;
    int voiceDlGain = -120;
    int voiceMmGain = -120;
    int voiceToneGain = -120;

    propMgr.get((String8)Omap4ALSAManager::VOICEMEMO_VUL_GAIN, voiceUlGain);
    propMgr.get((String8)Omap4ALSAManager::VOICEMEMO_VDL_GAIN, voiceDlGain);
    propMgr.get((String8)Omap4ALSAManager::VOICEMEMO_MM_GAIN, voiceMmGain);
    propMgr.get((String8)Omap4ALSAManager::VOICEMEMO_TONE_GAIN, voiceToneGain);

    // conversion from properties to ABE HAL gains:
    // Voice call record gains properties:
    // value: -120dB..29dB step 1dB (-120 is mute)

    // ABE VXREC volumes
    //  index       gain            step
    //  0..149      -120dB..29dB    1dB
    voiceUlGain += 120;
    voiceDlGain += 120;
    voiceMmGain += 120;
    voiceToneGain += 120;

    if (voiceMmGain)
        control.set("Capture Mixer Media Playback", 1);
    else
        control.set("Capture Mixer Media Playback", 0, 0);
    if (voiceToneGain)
        control.set("Capture Mixer Tones", 1);
    else
        control.set("Capture Mixer Tones", 0, 0);

    control.set("VXREC Media Volume", voiceMmGain);
    control.set("VXREC Tones Volume", voiceToneGain);


    if (channels & AudioSystem::CHANNEL_IN_VOICE_UPLINK) {
        control.set("Capture Mixer Voice Capture", 1);
        control.set("Capture Mixer Voice Playback", 0, 0);
        control.set("VXREC Voice UL Volume", voiceUlGain);
        control.set("VXREC Voice DL Volume", 0 ,0);
    } else if (channels & AudioSystem::CHANNEL_IN_VOICE_DNLINK) {
        control.set("Capture Mixer Voice Capture", 0, 0);
        control.set("Capture Mixer Voice Playback", 1);
        control.set("VXREC Voice DL Volume", voiceDlGain);
        control.set("VXREC Voice UL Volume", 0, 0);
    } else if (channels & AudioSystem::CHANNEL_IN_VOICE_UPLINK_DNLINK) {
        control.set("Capture Mixer Voice Capture", 1);
        control.set("Capture Mixer Voice Playback", 1);
        control.set("VXREC Voice UL Volume", voiceUlGain);
        control.set("VXREC Voice DL Volume", voiceDlGain);
    }
}
}
