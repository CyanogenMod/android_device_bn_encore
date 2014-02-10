#!/system/bin/sh

# configure_vold.sh -- set up vold depending on boot location

FSTAB=/fstab.encore
FSTAB_NEW=/dev/fstab.encore

PATH=/sbin:/system/bin:/system/xbin
umask 0022

bootdevice="`getprop ro.boot.bootdevice`"

# Configure vold to find /sdcard partition
case "$bootdevice" in
	SD )
		# Get last partition on SD card
		last_partition="`(cd /dev/block; ls mmcblk1p*) | sort -rn | head -n 1`"
		partnum="${last_partition##mmcblk1p}"

		# Update vold configuration for sdcard1
		sed -e "s/voldmanaged=sdcard1:auto/voldmanaged=sdcard1:$partnum,nonremovable/" "$FSTAB" > "$FSTAB_NEW"
		;;
	* )
		# No action required
		;;
esac

# Mount modified fstab over the original so that vold will find it
if [ -f "$FSTAB_NEW" ]; then
	mount -t bind -o bind "$FSTAB_NEW" "$FSTAB"
fi

: > /dev/.vold_configured

exit 0
