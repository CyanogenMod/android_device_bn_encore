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
#include <stdio.h>
#include <errno.h>

#define LOG_TAG "Encore PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define PIN_CORE_CLK_CTRL "/sys/devices/platform/omap/pvrsrvkm.0/pin_core_clk"
#define KSM_CTRL "/sys/kernel/mm/ksm/run"

static int enable_ksm = 1;

static int ksm_scanning_enabled(void) {
    FILE *fp;
    int value;

    /* Check to see whether KSM is enabled */
    if (!(fp = fopen(KSM_CTRL, "r"))) {
        ALOGW("Failed to open " KSM_CTRL ": %d", errno);
        return 0;
    }

    if ((value = fgetc(fp)) == EOF) {
        ALOGW("Failed to read current KSM state");
        value = '0';
    }
    fclose(fp);

    return (value != '0');
}

static void set_ksm_scanning(int on) {
    FILE *fp;

    ALOGI("Turning KSM scanning %s", on ? "on" : "off");
    if (!(fp = fopen(KSM_CTRL, "w"))) {
        ALOGW("Failed to open " KSM_CTRL ": %d", errno);
        return;
    }
    fprintf(fp, "%d\n", on ? 1 : 0);
    fclose(fp);
}

static void encore_power_init(struct power_module *module)
{
    enable_ksm = ksm_scanning_enabled();
}

static void encore_power_set_interactive(struct power_module *module, int on)
{
    FILE *fp;

    /* Enable/disable pinning the memory bus clock */
    ALOGI("Setting pin_core_clk to %d", on);
    if (!(fp = fopen(PIN_CORE_CLK_CTRL, "w"))) {
        ALOGE("Failed to open " PIN_CORE_CLK_CTRL ": %d", errno);
        return;
    }
    fprintf(fp, "%d\n", on ? 1 : 0);
    fclose(fp);

    if (!on) {
        /* Screen turning off */
        /* If KSM scanning is currently enabled, disable it and make a note
         * to reenable it once the screen comes back on */
        if (ksm_scanning_enabled()) {
            enable_ksm = 1;
            set_ksm_scanning(0);
        } else {
            enable_ksm = 0;
        }
    } else {
        /* Screen turning on */
        /* Reenable KSM scanning if it was enabled when the screen was last
         * turned off */
        if (enable_ksm) {
            set_ksm_scanning(1);
        }
    }
}

static void encore_power_hint(struct power_module *module, power_hint_t hint,
                       void *data) {
    switch (hint) {
    default:
        break;
    }
}

static struct hw_module_methods_t encore_power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Encore Power HAL",
        .author = "NookieDevs",
        .methods = &encore_power_module_methods,
    },

    .init = encore_power_init,
    .setInteractive = encore_power_set_interactive,
    .powerHint = encore_power_hint,
};
