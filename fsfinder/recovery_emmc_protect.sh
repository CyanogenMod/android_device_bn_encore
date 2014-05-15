#!/sbin/sh

# recovery_emmc_protect.sh -- when recovery is managing an SD card install,
# prevent direct modification of the corresponding eMMC partitions
#
# This is intended to protect users who accidentally attempt to flash older
# OTA packages which aren't compatible with the new eMMC/SD-agnostic partition
# naming scheme.

# If the recovery isn't managing SD card, do nothing
SYSTEM_DEV="`readlink -f /dev/block/by-name/system`"
[ x"$SYSTEM_DEV" != x"/dev/block/mmcblk1p2" ] && exit 0

EMMC_PROTECTED_PARTITIONS="mmcblk0p1 mmcblk0p5 mmcblk0p6"

case "$1" in
	start )
		for part in $EMMC_PROTECTED_PARTITIONS; do
			mount -t bind -o bind /dev/null /dev/block/"$part"
		done
		;;
	stop )
		for part in $EMMC_PROTECTED_PARTITIONS; do
			umount /dev/block/"$part"
		done
		;;
	* )
		echo "Usage: $0 {start|stop}"
		;;
esac

exit 0
