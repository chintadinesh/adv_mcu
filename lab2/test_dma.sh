#! /bin/bash

echo "initial cdma regs"
dm 0xb0000000 12

echo "\n"

echo "setting up cdma"
pm 0xb0000000 0x04

echo "\n"

echo "cdma regs after resetting"
dm 0xb0000000 12

echo "\n"

echo "preforming dma operation"
pm 0xfffc0000 0xabcdabcd
echo "\n"

pm 0xb0000018 0xfffc0000
pm 0xb0000020 0xb0028000
pm 0xb0000028 0x4

echo "\n"
dm 0xb0000004

echo "\n"

echo "testing bram memory"
dm 0xa0028000

