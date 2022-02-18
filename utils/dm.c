/*
 *  dm.c
 *
 *  author: Mark McDermott 
 *  Created: Feb 12, 2012
 *
 *  Simple utility to allow the use of the /dev/mem device to display memory
 *  and write memory
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
 
#include "stdio.h"
#include "stdlib.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char * argv[]) {



	unsigned int *regs;
	unsigned int *address;
	
	unsigned int target_addr, offset, temp_x, lp_cnt, page_cnt, page_num;
	
	int fd = open("/dev/mem", O_RDWR|O_SYNC, S_IRUSR);

	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

	if ((argc != 2) && (argc != 3) && (argc !=4))
	{
		printf("USAGE:  dm (address) (repeat) (#pages) :\n");
		close(fd);
		return -1;
	}
		
	offset = 0;
	//target_addr = strtoul(argv[1], 0, 0);
	
	target_addr = (strtoul(argv[1], 0, 0) & ~0x3);  // Align to 32 bit words
	//printf("Target ADDR: 0x%.4x\n" , (target_addr));
    
    lp_cnt = 1; // Display at least 1 locations
    
    //if (argc == 3) 	lp_cnt = strtoul(argv[2], 0, 0);
    //if (lp_cnt > 0x3ff) lp_cnt = 0x3ff;
    
    //lp_cnt += 1;
	
	//temp = target_addr & 0xfff;
	//temp = 0xfff - temp;
	
	page_cnt = 1;   
    if ((argc == 4)) page_cnt = (strtoul(argv[3], 0, 0));
    
    //page_cnt = (lp_cnt >> 12) + 1;
    //printf("Page CNT: 0x%.4x\n" , (page_cnt));
   
    page_num = 1;
    
    while (page_cnt) 
    {
        if ((argc == 3) || (argc ==4))  lp_cnt = strtoul(argv[2], 0, 0);
        if (lp_cnt >= 0x3ff) lp_cnt = 0x3ff;
        temp_x = target_addr & 0xfff;
	    temp_x = (0x1000 - temp_x)>>2; // 
	    //printf("temp = 0x%.4x\n" , (temp_x));      
	    if (temp_x < lp_cnt) lp_cnt = temp_x;
        
        regs = ( unsigned int *)mmap(NULL, 
                                 MAP_SIZE, 
                                 PROT_READ|PROT_WRITE, 
                                 MAP_SHARED, 
                                 fd, 
                                 target_addr & ~MAP_MASK);	
    
 	    //printf("Page number 0x%.4x\n" , (page_num));      

        
        while(lp_cnt) 
        {
	

	        //printf("0x%.2x" , (target_addr >> 12));
	        //printf("_%.4x" , (offset));
	        printf("0x%.4x" , (target_addr+offset));

            address = regs + (((target_addr+ offset) & MAP_MASK)>>2);    	
	
	        printf(" = 0x%.8x\n", *address);		// display register value
	        //printf(" = 0x%.8x", *address);		// display register value
	        //printf("_%.4x\n" , (offset));
	    
	        lp_cnt    -= 1;
	        offset    += 4; // WORD alligned
	        //printf("lp_cnt = 0x%.4x\n" , (lp_cnt));
	    } // End inner while loop
	
	    target_addr = target_addr + 4096;
	    offset = 0;
	
	    //printf("0x%.4x\n" , (target_addr + offset));
	    page_cnt -= 1;
        page_num += 1;
        
        munmap(NULL, MAP_SIZE); 
	    	
	}  // End of outer while loop	
	
	int temp = close(fd);
	if(temp == -1)
	{
		printf("Unable to close /dev/ram1.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

    //munmap(NULL, MAP_SIZE);
	
	return 0;
}
