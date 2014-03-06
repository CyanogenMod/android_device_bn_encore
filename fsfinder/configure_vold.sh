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
		partnum="`for i in /dev/block/mmcblk1p*; do echo ${i##/dev/block/mmcblk1p}; done | sort -rn | head -n 1`"

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

	# If in recovery, replace /etc/recovery.fstab too
	if [ -e /etc/recovery.fstab ]; then
		mount -t bind -o bind "$FSTAB_NEW" /etc/recovery.fstab
	fi
fi

: > /dev/.vold_configured

exit 0
