#!system/bin/sh

# clear the bootcount
dd if=/dev/zero of=/rom/devconf/bcb bs=1c count=4

