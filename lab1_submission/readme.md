
Please run the tests as exemplified below

Note: We did not make the clock modification atomic.
Because of this if you interrupt the process in the middle of changing PS
clock, the system might hang. To avoid the problem, give the word size to be
big. Please restart the system if that happens.

```
root@EE382N4:~/adv_mcu/lab1_submission# sudo bash
root@EE382N4:~/adv_mcu/lab1_submission# ls
DMA_OCMtest.c  Makefile  ultra96v2_oob_wrapper.bit
root@EE382N4:~/adv_mcu/lab1_submission# fpgautil -b ultra96v2_oob_wrapper.bit 
Time taken to load BIN is 3719.000000 Milli Seconds
BIN FILE loaded through zynqMP FPGA manager successfully
root@EE382N4:~/adv_mcu/lab1_submission# make
/usr/bin/gcc  -D DEBUG=0 -D TEST=1 -o test1 DMA_OCMtest.c
/usr/bin/gcc  -D DEBUG=0 -D TEST=2 -o test2 DMA_OCMtest.c
/usr/bin/gcc  -D DEBUG=0 -D TEST=3 -o test3 DMA_OCMtest.c
root@EE382N4:~/adv_mcu/lab1# ./test1
test1
^CCaught signal 2
Test passed: 33 loops of 1647633 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# ./test1 --loops 10
The arguments supplied are: loops = 10, size = -1
test1
Test passed: 10 loops of 512000 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# 
root@EE382N4:~/adv_mcu/lab1# ./test1 --loops 10 --words 200
The arguments supplied are: loops = 10, size = 200
test1
Test passed: 10 loops of 50000 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# 
root@EE382N4:~/adv_mcu/lab1# ./test2
test2
^CCaught signal 2
Test passed: 5 loops of 447542 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# ./test2 --loops 10
The arguments supplied are: loops = 10, size = -1
test2
Test passed: 10 loops of 921600 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# ./test2 --loops 10 --words 100
The arguments supplied are: loops = 10, size = 100
test2
Test passed: 10 loops of 22500 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# 
root@EE382N4:~/adv_mcu/lab1# 
root@EE382N4:~/adv_mcu/lab1# ./test3
test3
^CCaught signal 2
Test passed: 22 loops of 541696 32-bit words!!
root@EE382N4:~/adv_mcu/lab1# 
```
