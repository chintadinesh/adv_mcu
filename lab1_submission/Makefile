
DEBUG=0

CC := /usr/bin/gcc 

all: test1 test2 test3

test1: 
	$(CC) -D DEBUG=$(DEBUG) -D TEST=1 -o test1 DMA_OCMtest.c

test2:
	$(CC) -D DEBUG=$(DEBUG) -D TEST=2 -o test2 DMA_OCMtest.c

test3:
	$(CC) -D DEBUG=$(DEBUG) -D TEST=3 -o test3 DMA_OCMtest.c

clean::
	$(RM) test*

