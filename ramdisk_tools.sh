#!/bin/bash

# ramdisk_tools.sh
#
# (C) 2010-2011 fattire
# (C) 2011 Gerad Munsch <gmunsch@unforgivendevelopment.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# So what does this do? It creates an encore initramfs (ramdisk) from
# the files provided in ./root/. It automatically replaces the mount
# points in init.encore.rc, based on which argument is passed to the
# script. It supports creating ramdisks for internal, SD card, and
# alternate internal boot setups.
#
# Run this file script from $OUT -- (basically) the place you found it.
#
# Arguments:
# --internal  : creates ramdisk-internal.img for use on standard internal
#              emmc partitions
#
# --sdcard    : creates ramdisk-sdcard.img for use on SD card setups
#
# --alternate : create ramdisk-alternate.img for use on the alternate
#               internal emmc partitions (9,10,11)
#
# Place the resultant file in the appropriate /boot partition, with
# the appropriate name, reboot, and enjoy!

### CONSTANTS ###

# files
ROOT="root"
INIT="root/init.encore.rc"
INT_OUT="ramdisk-internal.img"
SD_OUT="ramdisk-sdcard.img"
ALT_OUT="ramdisk-alternate.img"

# partitions
INT_SYSTEM="mmcblk0p5"
INT_USERDATA="mmcblk0p6"
INT_CACHE="mmcblk0p7"
SD_SYSTEM="mmcblk1p2"
SD_USERDATA="mmcblk1p3"
SD_CACHE="mmcblk1p5"
ALT_SYSTEM="mmcblk0p9"
ALT_USERDATA="mmcblk0p10"
ALT_CACHE="mmcblk0p11"

# strings
INT_NAME_1="internal"
INT_NAME_2="emmc"
SD_NAME_1="SD"
SD_NAME_2="card"
ALT_NAME_1="alternate"
ALT_NAME_2="internal/emmc"
INT_BOOT="mmcblk0p1"
SD_BOOT="mmcblk1p1"
ALT_BOOT="mmcblk0p1"
INT_RAMDISK_NAME="uRamdisk"
SD_RAMDISK_NAME="uRamdisk"
ALT_RAMDISK_NAME="uAltRam"

### END CONSTANTS ###
### FUNCTION DEFINITIONS ###

## display help information
function display_help {
    echo -e "\033[1m$0\033[0m\n"
    echo -e "This is a combination script, used for building ramdisks for use with the Nook Color. It covers"
    echo -e "use with standard emmc, alternate emmc, and SD card partition setups. It uses the ./root/ folder"
    echo -e "for its input. The init file, \033[1minit.encore.rc\033[0m will automatically be modified to contain the"
    echo -e "appropriate mount points.\n"
    echo -e "\033[1mUSAGE:\033[0m"
    echo -e "\033[1m$0 --internal\033[0m  - rebuilds your ramdisk, setup for the default internal emmc partitions.\n    Output: \033[1mramdisk-internal.img\033[0m"
    echo -e "\033[1m$0 --sdcard\033[0m    - rebuilds your ramdisk, setup for installations on MicroSD cards.\n    Output: \033[1mramdisk-sdcard.img\033[0m"
    echo -e "\033[1m$0 --alternate\033[0m - rebuilds your ramdisk, setup for the alternate internal emmc partitions.\n    Output: \033[1mramdisk-alternate.img\033[0m"
    return 0
} # end function 'help_info' #

## check for existance of mkimage
function check_mkimage {
    mkimage=`which mkimage`
    if [ -z $mkimage ]; then
        echo -e "\n\033[1mY U NO have mkimage??\033[0m\n"
        echo -e "Please install mkimage from your distributions repositories, or elsewhere.\n"
        echo -e "Older Debian/Ubuntu: \033[1m(sudo) apt-get install uboot-mkimage\033[0m"
        echo -e "Newer Debian/Ubuntu: \033[1m(sudo) apt-get install u-boot-tools\033[0m\n"
        echo -e "Please try again once mkimage is accessible from your path/shell, and try again."
        return 1
    else
        echo -e "\n\033[1m* Found mkimage, proceeding.\033[0m"
        return 0
    fi
} # end function 'check_mkimage' #

## update init.encore.rc with proper partition names
function update_init {
    sed -i 's/.*mount.*\/system.*/    mount ext4 \/dev\/block\/'$1' \/system wait ro barrier=1/' $INIT
    sed -i 's/.*mount.*\/data.*/    mount ext4 \/dev\/block\/'$2' \/data wait noatime nosuid nodev barrier=1 noauto_da_alloc/' $INIT
    sed -i 's/.*mount.*\/cache.*/    mount ext4 \/dev\/block\/'$3' \/cache wait noatime nosuid nodev barrier=1/' $INIT
} # end function 'update_init' #

## create new ramdisk from ./root/ folder
function create_ramdisk {
    # enter initramfs (ramdisk) root
    cd $ROOT

    # collect files to be added to initramfs (ramdisk), and add them
    echo -e "\n\033[1m* Adding files:\033[0m"
    find . -regex "./.*" | cpio -ov -H newc | gzip > ../repacked-ramdisk.cpio.gz

    # create new initramfs (ramdisk) image using mkimage (u-boot-tools)
    echo -e "\n\033[1m* Creating ramdisk:\033[0m"
    mkimage -A ARM -T RAMDisk -n Image -d ../repacked-ramdisk.cpio.gz ../$1
} # end function 'create_ramdisk' #

## cleanup our mess
function cleanup_mess {
    rm ../repacked-ramdisk.cpio.gz
} # end function 'cleanup_mess' #

## display helpful info
function output_info {
    echo -e "\n\033[1m* Created ramdisk for $1 $2 boot. To install this ramdisk on your device, boot into your NC, then:\033[0m"
    echo -e "adb shell"
    echo -e "mount -o rw,remount /"
    echo -e "mkdir -p /mnt/boot"
    echo -e "mount -t vfat /dev/block/$3 /mnt/boot"
    echo -e "exit"
    echo -e "adb push $4 /mnt/boot/$5"
    echo -e "adb shell sync"
    echo -e "adb reboot"
} # end function 'output_info' #

## display extra info for alternate boot
function alt_boot_info {
    echo -e "\n\033[1m* Please keep in mind, you need uAltImg in place as well.\033[0m"
    echo "To set the default boot device to alternate eMMC partitions, do:"
    echo "adb shell"
    echo "mount -o rw,remount /rom"
    echo "cd /rom"
    echo "touch u-boot.altboot"
    echo "echo 1 > u-boot.altboot"
    echo "sync"
    echo "mount -o ro,remount /rom"
    echo -e "\n\033[1m* Your alternate boot image/ramdisk are now the default.\033[0m"
} # end function 'alt_boot_info' #

### END FUNCTION DEFINITIONS ###
### MAIN PROGRAM ###

# check for arguments -- if none exist, display "help" information
if [ $# -lt 1 ]; then
    display_help
fi

case $1 in

--internal)
    # rebuild ramdisk for use on standard emmc partitions
    check_mkimage
    create_ramdisk $INT_OUT
    cleanup_mess
    output_info $INT_NAME_1 $INT_NAME_2 $INT_BOOT $INT_OUT $INT_RAMDISK_NAME
;; # end case '--internal' #

--sdcard)
    # rebuild ramdisk for use on SD card
    check_mkimage
    update_init $SD_SYSTEM $SD_USERDATA $SD_CACHE
    create_ramdisk $SD_OUT
    cleanup_mess
    output_info $SD_NAME_1 $SD_NAME_2 $SD_BOOT $SD_OUT $SD_RAMDISK_NAME
;; # end case '--sdcard' #

--alternate)
    # rebuild ramdisk for use on alternate emmc partitions
    check_mkimage
    update_init $ALT_SYSTEM $ALT_USERDATA $ALT_CACHE
    create_ramdisk $ALT_OUT
    cleanup_mess
    output_info $ALT_NAME_1 $ALT_NAME_2 $ALT_BOOT $ALT_OUT $ALT_RAMDISK_NAME
    alt_boot_info
;; # end case '--alternate' #

--help)
    # display help
    display_help
;; # end case '--help' #

esac

### END MAIN PROGRAM ###
