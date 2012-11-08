/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_TAG "TI OMAP PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define CPUFREQ_INTERACTIVE "/sys/devices/system/cpu/cpufreq/interactive/"
#define CPUFREQ_CPU0 "/sys/devices/system/cpu/cpu0/cpufreq/"
#define BOOSTPULSE_PATH (CPUFREQ_INTERACTIVE "boostpulse")

#define MAX_FREQ_NUMBER 10
#define NOM_FREQ_INDEX 2

static int freq_num;
static char *freq_list[MAX_FREQ_NUMBER];
static char *max_freq, *nom_freq;
char current_max_freq[10];

struct omap_power_module {
    struct power_module base;
    pthread_mutex_t lock;
    int boostpulse_fd;
    int boostpulse_warned;
    int inited;
};

static int str_to_tokens(char *str, char **token, int max_token_idx) {
    char *pos, *start_pos = str;
    char *token_pos;
    int token_idx = 0;

    if (!str || !token || !max_token_idx) {
        return 0;
    }

    do {
        token_pos = strtok_r(start_pos, " \t\r\n", &pos);

        if (token_pos)
            token[token_idx++] = strdup(token_pos);
        start_pos = NULL;
    } while (token_pos && token_idx < max_token_idx);

    return token_idx;
}

static void sysfs_write(char *path, char *s) {
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

static int sysfs_read(char *path, char *s, int s_size) {
    char buf[80];
    int len, i;
    int fd;

    if (!path || !s || !s_size) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return fd;
    }

    len = read(fd, s, s_size-1);
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error reading from %s: %s\n", path, buf);
    } else {
        s[len] = '\0';
    }

    close(fd);
    return len;
}

static void omap_power_init(struct power_module *module) {
    struct omap_power_module *omap_device = (struct omap_power_module *) module;
    int tmp;
    char freq_buf[MAX_FREQ_NUMBER*10];

    tmp = sysfs_read(CPUFREQ_CPU0 "scaling_available_frequencies", freq_buf, sizeof(freq_buf));
    if (tmp <= 0) {
        return;
    }

    freq_num = str_to_tokens(freq_buf, freq_list, MAX_FREQ_NUMBER);
    if (!freq_num) {
        return;
    }

    max_freq = freq_list[freq_num - 1];
    tmp = (NOM_FREQ_INDEX > freq_num) ? freq_num : NOM_FREQ_INDEX;
    nom_freq = freq_list[tmp - 1];

    sysfs_write(CPUFREQ_INTERACTIVE "timer_rate", "20000");
    sysfs_write(CPUFREQ_INTERACTIVE "min_sample_time","60000");
    sysfs_write(CPUFREQ_INTERACTIVE "hispeed_freq", nom_freq);
    sysfs_write(CPUFREQ_INTERACTIVE "go_hispeed_load", "50");
    sysfs_write(CPUFREQ_INTERACTIVE "above_hispeed_delay", "100000");

    ALOGI("Initialized successfully");
    omap_device->inited = 1;
}

static int boostpulse_open(struct omap_power_module *omap_device) {
    char buf[80];

    pthread_mutex_lock(&omap_device->lock);

    if (omap_device->boostpulse_fd < 0) {
        omap_device->boostpulse_fd = open(BOOSTPULSE_PATH, O_WRONLY);

        if (omap_device->boostpulse_fd < 0) {
            if (!omap_device->boostpulse_warned) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGE("Error opening %s: %s\n", BOOSTPULSE_PATH, buf);
                omap_device->boostpulse_warned = 1;
            }
        }
    }

    pthread_mutex_unlock(&omap_device->lock);
    return omap_device->boostpulse_fd;
}

static void omap_power_set_interactive(struct power_module *module, int on) {
    struct omap_power_module *omap_device = (struct omap_power_module *) module;
    int tmp;

    if (!omap_device->inited)
        return;

    /*
     * Lower maximum frequency when screen is off.  CPU 0 and 1 share a
     * cpufreq policy.
     */

    // sysfs_write(CPUFREQ_CPU0 "scaling_max_freq", on ? max_freq : nom_freq);
    if (on) {
        sysfs_write(CPUFREQ_CPU0 "scaling_max_freq", (strlen(current_max_freq) > 0) ? current_max_freq : max_freq);
    } else {
        tmp = sysfs_read(CPUFREQ_CPU0 "scaling_max_freq", current_max_freq, sizeof(current_max_freq));
        if (tmp <= 0) {
            ALOGE("Error reading scaling_max_freq\n");
        }
        sysfs_write(CPUFREQ_CPU0 "scaling_max_freq", nom_freq);
    }
}

static void omap_power_hint(struct power_module *module, power_hint_t hint, void *data) {
    struct omap_power_module *omap_device = (struct omap_power_module *) module;
    char buf[80];
    int len;

    if (!omap_device->inited)
        return;

    switch (hint) {
    case POWER_HINT_INTERACTION:
        if (boostpulse_open(omap_device) >= 0) {
            len = write(omap_device->boostpulse_fd, "1", 1);

            if (len < 0) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGE("Error writing to %s: %s\n", BOOSTPULSE_PATH, buf);
            }
        }
        break;

    case POWER_HINT_VSYNC:
        break;

    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct omap_power_module HAL_MODULE_INFO_SYM = {
    .base = {
        .common = {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = POWER_MODULE_API_VERSION_0_2,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = POWER_HARDWARE_MODULE_ID,
            .name = "OMAP Power HAL",
            .author = "The Android Open Source Project",
            .methods = &power_module_methods,
        },

       .init = omap_power_init,
       .setInteractive = omap_power_set_interactive,
       .powerHint = omap_power_hint,
    },

    .lock = PTHREAD_MUTEX_INITIALIZER,
    .boostpulse_fd = -1,
    .boostpulse_warned = 0,
};
