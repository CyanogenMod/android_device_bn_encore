#!/bin/bash

# sd_ramdisk_packer.sh
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

# So what does this do?  It converts an encore /root "ramdisk" from
# supporting emmc to supporting SD.  Basically it just converts the
# init.encore.rc to point to the SD partitions as follows:
#
# /system == p2
# /data   == p3
# /cache  == p5
#
# run this script from $OUT -- basically the place you found it.  The
# resulting file "ramdisk.img" can be renamed "uRamdisk" and placed
# in mmcblk1p1 -- the vfat /boot partition.
#
# if you get an error about "mkimage" not being found, you'll need to
# . build/envsetup.sh

# Check for mkimage
# FIXME: This doesn't actually work. It returns OK even if mkimage
#        is not present at the shell.
if [ ! -f $(which mkimage) ]; then echo "Y U NO Have mkimage?"; exit 0; fi

# Define some stuff
ROOT=root
INIT=root/init.encore.rc

# Replace partition names in init
sed -i 's/mmcblk0p5/mmcblk0p9/' $INIT 
sed -i 's/mmcblk0p6/mmcblk0p10/' $INIT
sed -i 's/mmcblk0p7/mmcblk0p11/' $INIT

# Enter initramfs (ramdisk) root
cd $ROOT

# Collect files to be added to initramfs (ramdisk), and add them
find . -regex "./.*"| cpio -ov -H newc | gzip > ../repacked-ramdisk.cpio.gz 

# Create initramfs (ramdisk) image
mkimage  -A ARM -T RAMDisk -n Image -d ../repacked-ramdisk.cpio.gz ../ramdisk-alt.img

# Clean up our mess
rm ../repacked-ramdisk.cpio.gz

# Inform user what is going on, give instructions, etc..
echo -e "\n\n\033[1mCreated ramdisk for alternate boot. To install this ramdisk on your device, boot into your NC, then:\033[0m\n\n"
echo "adb shell"
echo "mount -o rw,remount /"
echo "mkdir -p /mnt/boot"
echo "mount -t vfat /dev/block/mmcblk0p1 /mnt/boot"
echo "exit"
echo "adb push ramdisk-alt.img /mnt/boot/uAltRam"
echo "adb shell sync"
echo "adb reboot"
echo -e "\n\n\033[1mPlease keep in mind, you need uAltImg in place as well.\033[0m"
echo "To set the default boot device to alternate eMMC partitions, do:"
echo "adb shell"
echo "mount -o rw,remount /rom"
echo "cd /rom"
echo "touch u-boot.altboot"
echo "echo 1 > u-boot.altboot"
echo "sync"
echo "mount -o ro,remount /rom"
echo -e "\n\n\033[1mYour alternate boot image/ramdisk are now the default.\033[0m"

exit
