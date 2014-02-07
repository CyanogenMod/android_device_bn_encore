/*
 * Copyright (C) 2014 Steven Luo
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

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CMDLINE_MAX 1024
#define BOOTDEVICE_PARAM "androidboot.bootdevice="
#define SYMLINKS_DIR "/dev/block/by-name/"

typedef enum {
	UNKNOWN,
	EMMC,
	SD,
} bootdevice_t;

static bootdevice_t parse_bootdevice_arg(char *arg) {
	size_t namelen = strlen(BOOTDEVICE_PARAM);
	if (strncmp(BOOTDEVICE_PARAM, arg, namelen) == 0) {
		arg += namelen;
		if (strcmp("eMMC", arg) == 0)
			return EMMC;
		else if (strcmp("SD", arg) == 0)
			return SD;
	}
	return UNKNOWN;
}

int main() {
	FILE *fp;
	char cmdline[CMDLINE_MAX];
	char *arg;
	bootdevice_t bootdev = UNKNOWN;

	if (!(fp = fopen("/proc/cmdline", "r"))) {
		fprintf(stderr, "Couldn't open kernel commandline!");
		goto do_symlinks;
	}
	if (!fgets(cmdline, CMDLINE_MAX, fp)) {
		fprintf(stderr, "Couldn't read kernel commandline!");
		fclose(fp);
		goto do_symlinks;
	}
	fclose(fp);

	/* Split command line into arguments and look for bootdev info */
	arg = strtok(cmdline, " ");
	if (arg)
		do {
			bootdev = parse_bootdevice_arg(arg);
		} while (bootdev == UNKNOWN && (arg = strtok(NULL, " ")));

do_symlinks:
	if (bootdev == UNKNOWN)
		fprintf(stderr, "WARNING: boot device not passed on kernel commandline");

	/* Set up the appropriate device symlinks for the boot device */
	switch (bootdev) {
	case SD:
		if (symlink("/dev/block/mmcblk1p1", SYMLINKS_DIR "boot") ||
		    symlink("/dev/block/mmcblk1p3", SYMLINKS_DIR "userdata") ||
		    symlink("/dev/block/mmcblk1p2", SYMLINKS_DIR "system")) {
			perror("Creating device symlink failed");
			return 1;
		}
		break;
	case EMMC:
	default:
		if (symlink("/dev/block/mmcblk0p1", SYMLINKS_DIR "boot") ||
		    symlink("/dev/block/mmcblk0p6", SYMLINKS_DIR "userdata") ||
		    symlink("/dev/block/mmcblk0p5", SYMLINKS_DIR "system")) {
			perror("Creating device symlink failed");
			return 1;
		}
		break;
	}

	return 0;
}
