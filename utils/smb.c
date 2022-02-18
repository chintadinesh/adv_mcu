/*
 *  smb.c
 *
 *  author: Mark McDermott 
 *  Created: Feb 12, 2012
 *  Modified Jun 6,  2020 to make fixes for 4.14 kernel
 *
 *  Simple utility to allow the use of the /dev/mem device to display memory
 *  and write single bits to a memory addresss
 
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 


/* -------------------------------------------------------------------------------  
 * One-bit masks for bits 0-31
 */

#define ONE_BIT_MASK(_bit)	(0x00000001 << (_bit))

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
	
	unsigned int target_addr;
        unsigned int pin_number;
	unsigned int bit_val;
	unsigned int reg_val;

	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	if (argc != 4)
	{
		printf("USAGE:  smb (address) (pin number) (data)  \n");
		close(fd);
		return -1;
	}
		
	target_addr = strtoul(argv[1], 0, 0);
	pin_number  = strtoul(argv[2], 0, 0);
	bit_val     = strtoul(argv[3], 0, 0);
	
	regs = (unsigned int *)mmap(NULL, 
                                    MAP_SIZE, 
                                    PROT_READ|PROT_WRITE, 
                                    MAP_SHARED, 
                                    fd, 
                                    target_addr & ~MAP_MASK
                                    );		
    
	 printf("0x%.8x" , (target_addr));

         address = regs + (((target_addr) & MAP_MASK)>>2);    	
    	
	 reg_val = *address;		     // Read regsiter value
	
         printf(" = 0x%.8x\n", *address);    // display register value

         printf("Mask = 0x%.8x\n", ONE_BIT_MASK(pin_number)); // Display mask value
	    
	 if (bit_val == 0) 
         {
        
              /* Deassert output pin in the target port's DR register*/
		
    	      reg_val &= ~ONE_BIT_MASK(pin_number);
	      *address = reg_val;
         } 
         else 
         {
	    
         /* Assert output pin in the target port's DR register*/
        	    
              reg_val |= ONE_BIT_MASK(pin_number);
	      *address = reg_val;
	 }	 
			
         printf(" = 0x%.8x\n", *address);    // display register value after write

  	 int temp = close(fd);
	 if(temp == -1)
	 {
		printf("Unable to close /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	 }	

	 munmap(NULL, MAP_SIZE);
	
         return 0;
}
