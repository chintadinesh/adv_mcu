#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

#include<signal.h>

//#define CDMA                0x40000000
#define CDMA                0xB0028000

//#define BRAM                0x43C00000
#define BRAM                0xA0028000

#define OCM                 0xFFFC0000

#define CDMACR              0x00
#define CDMASR              0x04

#define CURDESC_PNTR        0x08
#define CURDESC_PNTR_MSB    0x0C

#define TAILDESC_PNTR       0x10
#define TAILDESC_PNTR_MSB   0x14

#define SA                  0x18
#define SA_MSB              0x1C

#define DA                  0x20
#define DA_MSB              0x24

#define BTT                 0x28

#define PL_CTRL_REG        0xFF5E00C0 // we don't need offset

#define PS_CTRL_REG 0xFD1A0020

#define APLL_CTRL_OFFSET 0
#define APLL_CFG_OFFSET  0x24
#define APLL_STATUS_OFFSET 0x44

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

volatile unsigned int *regs, *address ;
volatile unsigned int target_addr, offset, value, lp_cnt;     

volatile unsigned int apll_ctrl_reg, apll_cfg_reg, pl_ref_ctrl_reg;

	 

/***************************  DMA SET ************************************
*   
*/

unsigned int dma_set(unsigned int* dma_virtual_address, int offset, unsigned int value) {
    dma_virtual_address[offset>>2] = value;
}

/***************************  DMA GET ************************************
*   
*/

unsigned int dma_get(unsigned int* dma_virtual_address, int offset) {
    return dma_virtual_address[offset>>2];
}

/***************************  CDMA SYNC ************************************
*   
*/

int cdma_sync(unsigned int* dma_virtual_address) {
    unsigned int status = dma_get(dma_virtual_address, CDMASR);
    if( (status&0x40) != 0)
    {
        unsigned int desc = dma_get(dma_virtual_address, CURDESC_PNTR);
        printf("error address : %X\n", desc);
    }
    while(!(status & 1<<1)){
        status = dma_get(dma_virtual_address, CDMASR);
    }
}

void memdump(void* virtual_address, int byte_count) {
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) { printf(" "); }
    }
    printf("\n");
}

void transfer_bram_to_ocm(unsigned int *cdma_virtual_address, int length)
{
    dma_set(cdma_virtual_address, DA, BRAM); // Write destination address
    dma_set(cdma_virtual_address, SA, OCM); // Write source address

    dma_set(cdma_virtual_address, BTT, length*4);

    cdma_sync(cdma_virtual_address);
}

//---------------------------our test codes----------------------
uint32_t* cdma_virtual_address;
uint32_t* BRAM_virtual_address;

int dh;
uint32_t* ocm;

uint32_t c[1024];


float ps_clks[5] = {1499, 1333, 999, 733, 416.6};
float pl_clks[5] = {300, 250, 187.5, 150, 100};

enum ps_clk_indices {IND_FBDIV, IND_DIV, IND_CP, IND_RES, IND_LFHF,
                        IND_LOCKDLY, IND_LOCKCNT};

int ps_clk_vals[5][7] = {
                            //1499
                        {
                            45, //        fbdiv = ,
                            0, //        div = ,

                            3, //        cp = ,
                            12, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            825, //        lockcnt = ,
                        },
                                //1333 => 
                        {
                            40, //        fbdif = ,
                            0, //        div = 

                            3, //        cp = ,
                            12, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            925, //        lockcnt = ,
                        },
                                //999 => 
                        {
                            30, //        
                            4, //        cp = ,
                            6, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            850, //        lockcnt = ,
                        },
                                //733 => 
                        {
                            44, //        ,
                            1, //        div = ,

                            3, //        cp = ,
                            12, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            850, //        lockcnt = ,
                        },
                         //       416 = 
                         {
                            25, //        , 
                            1, //        div = ,


                            3, //        cp = ,
                            10, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            1000 //        lockcnt = ,
    }
}



float pl_clks[5] = {300, 250, 187.5, 150, 100};

int pl_clk_divs[5] =  {
        5,
        6,
        8,
        10,
        15
    };

void set_random_ps_clk(int ind){
    /* % sudo dm 0xFD1A0020
     * 0xfd1a0020 = 0x00014800
     * 
     * Default freq:  33.3333MHz * 72 / 2 = 1199 MHz 
     */

     while (dm 0xfd1a0044 != 0x1) {;}// Pseudo code

    return;
}

void set_random_pl_clk(int ind){
    *pl_ref_ctrl_reg = (1<<24) // bit 24 enables clock
     | (1<<16) // bit 23:16 is divisor 1
      | (pl_clk_divs[ind] <<8); // bit 15:0 is clock divisor 0
      // frequency = 1.5Ghz/divisor0/divisor1
      // example = 1.5Ghz/6=250MHz
    return;
}

// Handler for SIGINT, caused by
// Ctrl-C at keyboard
void handle_sigint_test1(int sig)
{
    printf("Caught signal %d\n", sig);
    //clean the allocations here
    // unmap the memory locations

    munmap(ocm,65536);
    munmap(cdma_virtual_address,4096);
    munmap(BRAM_virtual_address,4096);
    munmap(pl_ref_ctrl_reg, 4096);

    exit(0);
}

// Handler for SIGINT, caused by
// Ctrl-C at keyboard
void handle_sigint_test2(int sig)
{
    printf("Caught signal %d\n", sig);
    // unmap the memory locations
    munmap(ocm,65536);
    munmap(cdma_virtual_address,4096);
    munmap(BRAM_virtual_address,4096);

    exit(0);
}


void test1(){

    signal(SIGINT, handle_sigint_test1);
	srand(time(0));         // Seed the ramdom number generator        
	                            		
    initilize();

    // RESET DMA
    // what does it mean to reset DMA
    dma_set(cdma_virtual_address, CDMACR, 0x04);

    while (1) {

        set_random_ps_clk(rand()%5);
        set_random_pl_clk(rand()%5);

	    value = rand();                 // Write random data
        offset = rand() % MAP_MASK;
        
        address = regs + (((target_addr + offset) & MAP_MASK)>>2);   
		*address = value; 			    // perform write command
	
        //printf("0x%.8x" , (target_addr + offset));
	    //printf(" = 0x%.8x\n", *address);// display register value
	    
	    //offset  += 4; 					// WORD alligned
    }
	  	    

    return;
}

void test2(){
    signal(SIGINT, handle_sigint_test2);
	srand(time(0));         // Seed the ramdom number generator        
	                            		
    initilize();

    // RESET DMA
    // what does it mean to reset DMA
    dma_set(cdma_virtual_address, CDMACR, 0x04);

    while (1) {

        set_random_ps_clk(rand()%5);
        //set_random_pl_clk(rand()%5);

	    value = rand();                 // Write random data
        offset = rand() % MAP_MASK;
        
        address = regs + (((target_addr + offset) & MAP_MASK)>>2);   
		*address = value; 			    // perform write command
	
        //printf("0x%.8x" , (target_addr + offset));
	    //printf(" = 0x%.8x\n", *address);// display register value
	    
	    //offset  += 4; 					// WORD alligned
    }
	  	    

    return;
}

void test3(){

    initialize();

    fill_ocm_with_random();
    
    // RESET DMA
    // what does it mean to reset DMA
    dma_set(cdma_virtual_address, CDMACR, 0x04);

//    printf("Source memory block:      "); memdump(virtual_source_address, 32);
//    printf("Destination memory block: "); memdump(virtual_destination_address, 32);

    // copy the content to bram from ocm
    transfer_bram_to_ocm(cdma_virtual_address, 1024);

    evaluate();
    return;
}

void initilize(){
    dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    cdma_virtual_address = mmap(NULL, 
                                4096, 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, 
                                dh, 
                                CDMA); // Memory map AXI Lite register block
    BRAM_virtual_address = mmap(NULL, 
                                4096, 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, 
                                dh, 
                                BRAM); // Memory map AXI Lite register block

    pl_ref_ctrl_reg = mmap(NULL,
                                0x1000,
                                PROT_READ|PROT_WRITE,
                                MAP_SHARED, 
                                dh, 
                                PL_CTRL_REG );

    apll_ctrl_reg = mmap(NULL,
                                0x1000,
                                PROT_READ|PROT_WRITE,
                                MAP_SHARED, 
                                dh, 
                                PS_CTRL_REG );


    printf("memory allocation\n");

    ocm = mmap(NULL, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, dh, OCM);

    return;
}

void evaluate(){

    //compare the values in bram with expected values 
    for(int i=0; i<20; i++)
    {
        if(BRAM_virtual_address[i] != c[i])
        {
            printf("RAM result: %d and c result is %d  element %d\n", BRAM_virtual_address[i], c[i], i);
            printf("test failed!!\n");
            munmap(ocm,65536);
            munmap(cdma_virtual_address,4096);
            munmap(BRAM_virtual_address,4096);
            return -1;
        }
    }
    printf("test passed!!\n");

    // unmap the memory locations
    munmap(ocm,65536);
    munmap(cdma_virtual_address,4096);
    munmap(BRAM_virtual_address,4096);

    return;
}


void fill_ocm_with_random(){
    //uint32_t c_t[20];

    // set the Onchip memory
    for(int i=0; i<20; i++){
        c[i] = rand();
        ocm[i] = c[i];
    }
}

int main() {

    initilize();

    
    // RESET DMA
    // what does it mean to reset DMA
    dma_set(cdma_virtual_address, CDMACR, 0x04);


    struct timeval start, end;

//    printf("Source memory block:      "); memdump(virtual_source_address, 32);
//    printf("Destination memory block: "); memdump(virtual_destination_address, 32);

    // copy the content to bram from ocm
    transfer_bram_to_ocm(cdma_virtual_address, 20);

    evaluate();

    return 0;
}

