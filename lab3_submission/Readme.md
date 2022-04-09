# please change to root user.
```
ultra96@EE382N4:~$ sudo bash
[sudo] password for ultra96:
```
# step1: please extract the tar file and load the bitstream.
```
root@EE382N4:~# tar -xzf dc47444_lab3.tar.gz
root@EE382N4:~# ls lab3_submission
adv_mcu_lab3.pdf  capture_timer_verilog  kernel_module  tests
app               fpga_files             plots
root@EE382N4:~#
root@EE382N4:~#
root@EE382N4:~/lab3_submission# ls
adv_mcu_lab3.pdf  capture_timer_verilog  kernel_module  Readme.md
app               fpga_files             plots          tests
root@EE382N4:~/lab3_submission# cd fpga_files/
root@EE382N4:~/lab3_submission/fpga_files# ls
system.dtb  system.dts  ultra96v2_oob_wrapper.bit
pper.bit 2N4:~/lab3_submission/fpga_files# fpgautil -b ultra96v2_oob_wrap
Time taken to load BIN is 3720.000000 Milli Seconds
BIN FILE loaded through zynqMP FPGA manager successfully
root@EE382N4:~/lab3_submission/fpga_files#
```
# step2: chdir to kernel_modules directory and build
```
root@EE382N4:~# cd lab3_submission/kernel_module/
root@EE382N4:~/lab3_submission/kernel_module# make
make -C /usr/src/plnx_kernel M=/home/ultra96/lab3_submission/kernel_module modules
make[1]: Entering directory '/usr/src/4.14.0-xilinx-v2018.3'
  CC [M]  /home/ultra96/lab3_submission/kernel_module/gpio_int.o
  CC [M]  /home/ultra96/lab3_submission/kernel_module/cdma_int.o
  Building modules, stage 2.
  MODPOST 2 modules
  CC      /home/ultra96/lab3_submission/kernel_module/cdma_int.mod.o
  LD [M]  /home/ultra96/lab3_submission/kernel_module/cdma_int.ko
  CC      /home/ultra96/lab3_submission/kernel_module/gpio_int.mod.o
  LD [M]  /home/ultra96/lab3_submission/kernel_module/gpio_int.ko
make[1]: Leaving directory '/usr/src/4.14.0-xilinx-v2018.3'
root@EE382N4:~/lab3_submission/kernel_module#
```
# step3: chdir to app and build
```
root@EE382N4:~/lab3_submission/kernel_module# cd ../app/
root@EE382N4:~/lab3_submission/app# make
/usr/bin/gcc  -g -D DEBUG=0 -D TEST1 -o test1 DMA_OCMtest.c
/usr/bin/gcc   intr_latency_ticks.c -o intr_latency_ticks
/usr/bin/gcc   intr_latency_ticks.c -g -o intr_latency_ticks_gdb
root@EE382N4:~/lab3_submission/app#
```
# step4: load the cdma kernel module for test1
```
root@EE382N4:~/lab3_submission/app# cd ../
root@EE382N4:~/lab3_submission# ls
adv_mcu_lab3.pdf  capture_timer_verilog  kernel_module  tests
app               fpga_files             plots
root@EE382N4:~/lab3_submission# cd kernel_module/
root@EE382N4:~/lab3_submission/kernel_module# ls
cdma_int.c      cdma_int.mod.o  gpio_int.ko     gpio_int.o     Module.symvers
cdma_int.ko     cdma_int.o      gpio_int.mod.c  Makefile       setup_cdma.sh
cdma_int.mod.c  gpio_int.c      gpio_int.mod.o  modules.order  setup.sh
root@EE382N4:~/lab3_submission/kernel_module# ./setup_cdma.sh
rm: cannot remove '/dev/cdma_int': No such file or directory
rmmod: ERROR: Module cdma_int is not currently loaded
root@EE382N4:~/lab3_submission/kernel_module#
```
# step5: chdir to app and run test1 using the run.py file.
This will run test1 with all the 9 possible frequency combinations.
After the run, 9 csv files are generated in rpts directory. I have used the
csv files for the plot shown in the report. Each test takes about 2-5 mins.
```
root@EE382N4:~/lab3_submission/kernel_module# cd ../app/
root@EE382N4:~/lab3_submission/app# ls
DMA_OCMtest.c         intr_latency_ticks_gdb  run_intr_latency.py
intr_latency_ticks    Makefile                run.py
intr_latency_ticks.c  rpts                    test1
root@EE382N4:~/lab3_submission/app# cat run.py
#! /usr/bin/python

import os

def run_tests():
    for i in range(3):
        for j in range(3):
            run_cmd = "../test1 --ps_freq " + str(i) + " --pl_freq " + str(j)
            print(">>> " + run_cmd)
            os.system(run_cmd)

if(not os.path.exists("rpts")):
    os.system("mkdir rpts");

os.chdir("rpts")

run_tests()
root@EE382N4:~/lab3_submission/app# ./run.py
...
...
```
# step6: chdir to kerne_modules and load gpio_int kernel module for measuring interrupt latencies.
```
root@EE382N4:~/lab3_submission/app# cd ../kernel_module/
root@EE382N4:~/lab3_submission/kernel_module#
root@EE382N4:~/lab3_submission/kernel_module# lsmod
Module                  Size  Used by
cdma_int               16384  0
wilc_sdio             118784  0
zynqmp_r5_remoteproc    16384  0
mali                  245760  0
uio_pdrv_genirq        16384  0
root@EE382N4:~/lab3_submission/kernel_module# ./setup.sh 
rm: cannot remove '/dev/gpio_int': No such file or directory
rmmod: ERROR: Module gpio_int is not currently loaded
root@EE382N4:~/lab3_submission/kernel_module# lsmod
Module                  Size  Used by
gpio_int               16384  0
cdma_int               16384  0
wilc_sdio             118784  0
zynqmp_r5_remoteproc    16384  0
mali                  245760  0
uio_pdrv_genirq        16384  0
root@EE382N4:~/lab3_submission/kernel_module# 
```
