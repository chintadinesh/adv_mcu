/*
 *  fm.c
 *
 *  author: Mark McDermott 
 *  Created: Feb 12, 2012
 *
 *  
 
     fm - fill memory with data
     USAGE:     fm (address) (write data) (#addresses) (data increment) 

     Example:   fm 0x40000000 0x55 0x5 0x1
                0x40000000 = 0x00000055
                0x40000004 = 0x00000056
                0x40000008 = 0x00000057
                0x4000000c = 0x00000058
                0x40000010 = 0x00000059
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
	volatile unsigned int target_addr, offset, value, lp_cnt, incr;

	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	
	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	if ((argc != 3) && (argc != 4) && (argc != 5))
	{
		printf("Fill Memory - USAGE:  fm (address) (write data) (#addresses) (data increment) \n");
		close(fd);
		return -1;
	}
		
	offset = 0;
	target_addr = strtoul(argv[1], 0, 0);   
    value       = strtoul(argv[2], 0, 0);    
    
    lp_cnt      = 1;        // Write at least 1 location
    incr        = 1;        // Set increment to ++1
    
    
    if (argc == 5)  {
        incr    = strtoul(argv[4], 0, 0);
        lp_cnt  = strtoul(argv[3], 0, 0);
        //printf ("lp_cnt = 0x%.8x\n" , (lp_cnt));
	    //printf ("incr =0x%.8x\n" , (incr));
	}
	
	if (argc == 4) {
        lp_cnt  = strtoul(argv[3], 0, 0);
        //printf ("lp_cnt = 0x%.8x\n" , (lp_cnt));
    }
	
	if (lp_cnt > 0x3ff)  { // Max is 4096 bytes
        lp_cnt = 0x3ff; 
        printf("Setting max repeat value to 0x3ff\n");
        
    } 
    
	regs = (unsigned int *)mmap(NULL, 
	                            MAP_SIZE, 
	                            PROT_READ|PROT_WRITE, 
	                            MAP_SHARED, 
	                            fd, 
	                            target_addr & ~MAP_MASK);
	                            		
    /* ---------------------------------------------------------------
    *   Main loop
    */     			
    
    while (lp_cnt) {
    
        address = regs + (((target_addr + offset) & MAP_MASK)>>2);   
		*address = value; 			    // perform write command
	
        printf("0x%.8x" , (target_addr + offset));
	    printf(" = 0x%.8x\n", *address);// display register value
	    
	    value   = value + incr;         // increment value by incr
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
