/*
 *  pm.c
 *
 *  author: Mark McDermott 
 *  Created: Feb 12, 2012
 *  Modified Jun 6,  2020 to make fixes for 4.14 kernel
 * 
 *  Simple utility to allow the use of the /dev/mem device to display memory
 *  and write memory addresses
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

// ------------------------------------------------------------------------------
//
//  Main Routine
//
// -----------------------------------------------------------------------------

int main(int argc, char * argv[]) {


	
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	volatile unsigned int *regs, *address ;
	
//	unsigned long target_addr, offset, value, lp_cnt;
	unsigned int target_addr, offset, value, lp_cnt;

	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	if ((argc != 3) && (argc != 4))
	{
		printf("USAGE:  pm (address) (write data) (optional repeat #) \n");
		close(fd);
		return -1;
	}
		
	offset = 0;
	target_addr = strtoul(argv[1], 0, 0);
    lp_cnt = 1; // Display at least 1 location
    
    if (argc == 4) 	lp_cnt = strtoul(argv[3], 0, 0);
	
	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);		

    while (lp_cnt) {
    
	    printf("0x%.8x" , (target_addr + offset));

        address = regs + (((target_addr+ offset) & MAP_MASK)>>2);    	
    	
		value = strtoul(argv[2], 0, 0);
		*address = value; 						// perform write command
	
	    printf(" = 0x%.8x\n", *address);		// display register value
	    
	    lp_cnt -= 1;
	//    offset  += 4; 							// WORD alligned
	  	    
	  } // End of while loop
	  
			
	int temp = close(fd);
	if(temp == -1)
	{
		printf("Unable to close /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	munmap(NULL, MAP_SIZE);
	
	return 0;
}
