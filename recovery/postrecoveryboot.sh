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

# Symlink /system/bin to /sbin for compatibility with the reboot-recovery
# script and other shell scripts for the regular, non-recovery environment
rmdir /system/bin
ln -s /sbin /system/bin

# Align /sdcard and /emmc with their usage in the non-recovery environment
rmdir /emmc
ln -s /storage/sdcard0 /emmc
rmdir /sdcard
ln -s /storage/sdcard1 /sdcard
