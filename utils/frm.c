/*
 *  frm.c
 *
 *  author: Mark McDermott 
 *  Created: Feb 12, 2012
 *
      frm - Fill memory with random data 
      USAGE:  fm (address) (count)
    
      Example:  frm 0x40000000 0x5
                0x40000000 = 0x3d6af58a
                0x40000004 = 0x440be909
                0x40000008 = 0x25b16123
                0x4000000c = 0x648b26f3
                0x40000010 = 0x177985f0
 *
 */
 
#include "stdio.h"
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char * argv[]) {

    volatile unsigned int *regs, *address ;
	volatile unsigned int target_addr, offset, value, lp_cnt;     
    
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	
	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	if ((argc < 2))
	{
		printf("Fill Memory Random Data - USAGE:  frm (address) (# addresses) \n");
		close(fd);
		return -1;
	}
		
	lp_cnt = 1;        // Write at least 1 location
	offset = 0;
	
	target_addr = strtoul(argv[1], 0, 0);   
    
    value = 0;
    
    if (argc == 3) {
        lp_cnt  = strtoul(argv[2], 0, 0);
    }

    if (lp_cnt > 0x400)  { // Max is 4096 bytes
        lp_cnt = 0x400; 
        printf("Setting max repeat value to 0x3ff\n");
    }

	regs = (unsigned int *)mmap(NULL, 
	                            MAP_SIZE, 
	                            PROT_READ|PROT_WRITE, 
	                            MAP_SHARED, 
	                            fd, 
	                            target_addr & ~MAP_MASK);
	
	srand(time(0));         // Seed the ramdom number generator        
	                            		
    /* ---------------------------------------------------------------
    *   Main loop
    */     			
    
    while (lp_cnt) {
    
	    value = rand();                 // Write random data
        
        address = regs + (((target_addr + offset) & MAP_MASK)>>2);   
		*address = value; 			    // perform write command
	
        printf("0x%.8x" , (target_addr + offset));
	    printf(" = 0x%.8x\n", *address);// display register value
	    
	    lp_cnt  -= 1;                   // decrement loop
	    offset  += 4; 					// WORD alligned
	  	    
	  } // End of while loop
	 
    
    
    /* ---------------------------------------------------------------
    *   Clenup and Exit
    */    
    			
	int temp = close(fd);
	
	if(temp == -1)
	{
		printf("Unable to close /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	munmap(NULL, MAP_SIZE);
	
	return 0;
}
