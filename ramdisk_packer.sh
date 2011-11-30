#!/bin/bash

# ramdisk_packer.sh
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

# /system == p2
# /data   == p3
# /cache  == p5
#
# run this script from $OUT -- basically the place you found it.  The
# resulting file "ramdisk.img" can be renamed "uRamdisk" and placed
# in mmcblk0p1 -- the vfat /boot partition.
#
# if you get an error about "mkimage" not being found, you'll need to
# . build/envsetup.sh

if [ ! -f $(which mkimage) ]; then echo "Y U NO Have mkimage?"; exit 0; fi

ROOT=root
INIT=root/init.encore.rc

cd $ROOT
find . -regex "./.*"| cpio -ov -H newc | gzip > ../repacked-ramdisk.cpio.gz 
mkimage  -A ARM -T RAMDisk -n Image -d ../repacked-ramdisk.cpio.gz ../ramdisk.img
rm ../repacked-ramdisk.cpio.gz
echo -e "\n\nTo install this ramdisk on your device, boot into your NC, then:\n\n"
echo "adb shell"
echo "mkdir -p /data/mnt"
echo "mount -t vfat /dev/block/mmcblk0p1 /data/mnt"
echo "exit"
echo "adb push ramdisk.img /data/mnt/uRamdisk"
echo "adb reboot"
exit
