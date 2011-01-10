#!system/bin/sh

# clear the bootcount
dd if=/dev/zero of=/rom/devconf/BootCnt bs=1c count=4

