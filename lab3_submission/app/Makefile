
DEBUG=0

CC := /usr/bin/gcc 

all: test1 intr_latency_ticks 

test1: DMA_OCMtest.c
	$(CC) -g -D DEBUG=$(DEBUG) -D TEST1 -o test1 DMA_OCMtest.c

intr_latency_ticks: intr_latency_ticks.c
	$(CC)  intr_latency_ticks.c -o intr_latency_ticks
	$(CC)  intr_latency_ticks.c -g -o intr_latency_ticks_gdb

clean::
	$(RM) test*
	$(RM) intr_latency_gdb intr_latency_csv 
	$(RM) intr_latency_ticks intr_latency_ticks_gdb 

