#!/system/bin/sh

# log battery data
while :;
do
  cat /sys/bus/i2c/drivers/max17042/1-0036/histdata > /rom/max17042.bin
  sleep 3600
done

