#!/sbin/sh

# Resets the boot counter and the bcb instructions
mkdir /rom
mount /dev/block/mmcblk0p2 /rom
mount -o rw,remount /rom

# Zero out the boot counter
dd if=/dev/zero of=/rom/devconf/BootCnt bs=1 count=4

# Reset the bootloader control block (bcb) file
dd if=/dev/zero of=/rom/bcb bs=1 count=1088

umount /rom
rmdir /rom

