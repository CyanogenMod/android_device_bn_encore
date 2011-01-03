#!/sbin/sh

mount /dev/block/mmcblk0p7 /cache

# make dirs for custom mounts
mkdir /rom
mkdir /factory

# clear recovery flags and rest boot count for u-boot
mount /dev/block/mmcblk0p2 /rom
echo -n -e "\x00\x00\x00\x00" > /rom/devconf/BootCnt 
echo -n -e "\x00\x00\x00\x00" > /rom/bcb
umount /rom
