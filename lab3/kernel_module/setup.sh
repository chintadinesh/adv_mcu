#!/bin/sh

rm /dev/gpio_int
/bin/mknod /dev/gpio_int c 240 0

/sbin/rmmod gpio_int
/sbin/insmod gpio_int.ko


#/bin/pm 0x43c00000 0x55 > /dev/null
#sleep 1.0
#/bin/pm 0x43c00000 0xaa > /dev/null
#sleep 1.0
#cat /proc/interrupts | grep gpio

#while (ls > /dev/null) do ./intr_latency.exe; cat /proc/interrupts | grep gpio; done

