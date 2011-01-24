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

if [ ! -f $(which mkimage) ]; then echo "Y U NO Have mkimage?"; exit 0; fi

ROOT=root
INIT=root/init.encore.rc

sed -i '' 's/mmcblk0p5/mmcblk1p2/' $INIT 
sed -i '' 's/mmcblk0p6/mmcblk1p3/' $INIT
sed -i '' 's/mmcblk0p7/mmcblk1p5/' $INIT
cd $ROOT
find . -regex "./.*"| cpio -ov -H newc | gzip > ../repacked-ramdisk.cpio.gz 
mkimage  -A ARM -T RAMDisk -n Image -d ../repacked-ramdisk.cpio.gz ../ramdisk.img
rm ../repacked-ramdisk.cpio.gz
echo -e "\n\nTo install this ramdisk on your device, boot into your NC, then:\n\n"
echo "adb shell"
echo "mount -o rw,remount /"
echo "mkdir -p /mnt/boot"
echo "mount /mnt/block/mmcblk1p1 /mnt/boot"
echo "exit"
echo "adb push ramdisk.img /mnt/boot/uRamdisk"
echo "adb reboot"
exit
