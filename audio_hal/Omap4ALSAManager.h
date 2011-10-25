/* Omap4ALSAManager.h
 **
 ** Copyright 2011-2012 Texas Instruments
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

#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>
#include <stdlib.h>

namespace android
{

class String8;

class Omap4ALSAManager
{
    public:
        Omap4ALSAManager() {}
        virtual ~Omap4ALSAManager();

        status_t remove(const String8& key);
        status_t get(const String8& key, String8& value);
        status_t get(const String8& key, int& value);

        status_t set(const String8& key, const String8& value);
        status_t setFromProperty(const String8& key);
        status_t setFromProperty(const String8& key, const String8& init);

        status_t validateValueForKey(const String8& key, String8& value);

        // the keys
        static const char* MAIN_MIC;
        static const char* SUB_MIC;
        static const char* POWER_MODE;
        static const char* DL2L_EQ_PROFILE;
        static const char* DL2R_EQ_PROFILE;
        static const char* DL1_EQ_PROFILE;
        static const char* AMIC_EQ_PROFILE;
        static const char* DMIC_EQ_PROFILE;
        static const char* SDT_EQ_PROFILE;
        static const char* VOICEMEMO_VUL_GAIN;
        static const char* VOICEMEMO_VDL_GAIN;
        static const char* VOICEMEMO_MM_GAIN;
        static const char* VOICEMEMO_TONE_GAIN;
        static const char *DL1_EAR_MONO_MIXER;
        static const char *DL1_HEAD_MONO_MIXER;
        static const char *DL2_SPEAK_MONO_MIXER;
        static const char *DL2_AUX_MONO_MIXER;

        // list of properties per devices
        KeyedVector <String8, String8> mParams;

        size_t size() { return mParams.size(); }

        static const char  *MicNameList[];
        static const char  *PowerModeList[];
        static const char  *EqualizerProfileList[];

};


}; // namespace android
