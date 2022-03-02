
Please run the tests as exemplified below


```
root@EE382N4:~/adv_mcu/lab1_submission# ls
DMA_OCMtest.c  Makefile  ultra96v2_oob_wrapper.bit
root@EE382N4:~/adv_mcu/lab1_submission# fpgautil -b ultra96v2_oob_wrapper.bit 
Time taken to load BIN is 3719.000000 Milli Seconds
BIN FILE loaded through zynqMP FPGA manager successfully
root@EE382N4:~/adv_mcu/lab1_submission# make
/usr/bin/gcc  -D DEBUG=0 -D TEST=1 -o test1 DMA_OCMtest.c
/usr/bin/gcc  -D DEBUG=0 -D TEST=2 -o test2 DMA_OCMtest.c
/usr/bin/gcc  -D DEBUG=0 -D TEST=3 -o test3 DMA_OCMtest.c
root@EE382N4:~/adv_mcu/lab1_submission# ./test1 
test1
^CCaught signal 2
Test passed: 23 loops of 572416 32-bit words!!
root@EE382N4:~/adv_mcu/lab1_submission# ./test2 
test2
^CCaught signal 2
Test passed: 30 loops of 750592 32-bit words!!
root@EE382N4:~/adv_mcu/lab1_submission# ./test3
test3
^CCaught signal 2
Test passed: 34 loops of 860160 32-bit words!!
root@EE382N4:~/adv_mcu/lab1_submission# 
```
