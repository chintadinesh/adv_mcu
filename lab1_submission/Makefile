
DEBUG=0

CC := /usr/bin/gcc 

all: test1 test2 test3

test1: DMA_OCMtest.c
	$(CC) -g -D DEBUG=$(DEBUG) -D TEST1 -o test1 DMA_OCMtest.c

test2: DMA_OCMtest.c
	$(CC) -g -D DEBUG=$(DEBUG) -D TEST2 -o test2 DMA_OCMtest.c

test3: DMA_OCMtest.c
	$(CC) -g -D DEBUG=$(DEBUG) -D TEST3 -o test3 DMA_OCMtest.c

clean::
	$(RM) test*

