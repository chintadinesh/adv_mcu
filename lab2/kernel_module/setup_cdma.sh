#!/bin/sh

rm /dev/cdma_int
/bin/mknod /dev/cdma_int c 245 0

/sbin/rmmod cdma_int
/sbin/insmod cdma_int.ko


#/bin/pm 0x43c00000 0x55 > /dev/null
#sleep 1.0
#/bin/pm 0x43c00000 0xaa > /dev/null
#sleep 1.0
#cat /proc/interrupts | grep cdma

#while (ls > /dev/null) do ./intr_latency.exe; cat /proc/interrupts | grep cdma; done

