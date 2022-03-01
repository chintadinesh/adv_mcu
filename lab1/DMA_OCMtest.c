#include <errno.h>
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

#include <sys/types.h>
#include <sys/stat.h>

//#define CDMA                0x40000000
#define CDMA                0xB0000000

//#define BRAM                0x43C00000
#define BRAM_DMA            0xB0028000

#define BRAM                0xA0028000

#define OCM                 0xFFFC0000
#define OCM_BACK            0xFFFC2000

// AAHA: We are not supposed to any random addresses.
// The below address doesn't mean anything.
// Not sure why the above one is different though.
//#define OCM_BACK            0xFFFD0000

// CONTROL register
#define CDMACR              0x00
// STATUS register
#define CDMASR              0x04

#define CURDESC_PNTR        0x08
#define CURDESC_PNTR_MSB    0x0C

#define TAILDESC_PNTR       0x10
#define TAILDESC_PNTR_MSB   0x14

// Source Address register
#define SA                  0x18
#define SA_MSB              0x1C

// Destination Address register
#define DA                  0x20
#define DA_MSB              0x24

#define BTT                 0x28

// PL clock physicl address for mmap
#define PL_CTRL_REG        0xFF5E0000

// PS clock physical address for mmap
#define PS_CTRL_REG 0xFD1A0000

// offsets for control, config and status registers of PS clk
#define APLL_CTRL_OFFSET 0x20
#define APLL_CFG_OFFSET  0x24
#define APLL_STATUS_OFFSET 0x44

// useful page size and masks, while doing mmap
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#ifndef DEBUG
#define DEBUG 0
#endif

#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

// not sure. Icommented the code 
volatile unsigned int *regs, *address ;
volatile unsigned int target_addr, offset, value, lp_cnt;     

// virtual address for pl and ps regions
unsigned int *pl_ref_ctrl_reg, *ps_ref_ctrl_reg;

// virtual address for ps clk ctrl, cfg and status regs
volatile unsigned int *apll_ctrl_reg, *apll_cfg_reg, *apll_status_reg; 

// This is the gloal config varaible to which test is currentlty performing
int test_number = 1;
/***************************  DMA SET ************************************
*   
*/

int num_loops, num_words;

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

void transfer_dma(unsigned int *cdma_virtual_address, int length,
        unsigned int src_address, unsigned int dst_address)
{
    dma_set(cdma_virtual_address, SA, src_address); // Write source address
    dma_set(cdma_virtual_address, DA, dst_address); // Write destination address

    dma_set(cdma_virtual_address, BTT, length*4);

    cdma_sync(cdma_virtual_address);
}

//---------------------------our test codes----------------------
// virtual address for CDMA and 
// bram region 
unsigned int* cdma_virtual_address;

unsigned int* BRAM_virtual_address;

// file descriptor for memory 
int dh;

// virtual address for On chip memory
unsigned int* ocm;
unsigned int* ocm_back;

// temporory array for generating values. Not so significant.
unsigned int c[1024];


// used while printing. For reference during coding.
float ps_clks[5] = {1499, 1333, 999, 733, 416.6};
float pl_clks[5] = {300, 250, 187.5, 150, 100};

// Array to deal with ps clk parameters
enum ps_clk_indices {IND_FBDIV, IND_DIV, IND_CP, IND_RES, IND_LFHF,
                        IND_LOCKDLY, IND_LOCKCNT};

unsigned int ps_clk_vals[5][7] = {
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
                            0,

                            4, //        cp = ,
                            6, //        res = ,
                            3, //        lfhf = ,
                            63, //        lockdly = ,
                            1000, //        lockcnt = ,
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
};



int pl_clk_divs[] =  {
        5,
        6,
        8,
        10,
        15
    };

void set_random_ps_clk(int ind){
    /* 
    * % sudo dm 0xFD1A0020
    * 0xfd1a0020 = 0x00014800
    * 
    * Default freq:  33.3333MHz * 72 / 2 = 1199 MHz 
    */

    /* the seven steps mentioned in the lab
    * Set APLL_CTRL = 0x0000_2D00: [DIV2] = 0x0, [FBDIV] = 0X2D
            pm 0xfd1a0020 0x00002D00

    * Program the control data for APLL_CFG using the data in Table 1.
            Set APLL_CFG[31:25] = 0x3F // LOCK_DLY
            Set APLL_CFG[22:13] = 0x339 // LOCK_CNT
            Set APLL_CFG[11:10] = 0x3 // LFHF
            Set APLL_CFG[8:5] = 0x3 // CP
            Set APLL_CFG[3:0] = 0x12 // RES

    * Set APLL_CFG[31:0] = 0x7E67_2C6C
            pm 0xfd1a0024 0x7E672C6C

    * Program the bypass:
            Set APLL_CTRL[31:0] = 0x0000_2D08h: [BYPASS] = 0x1
            pm 0xfd1a0020 0x00002D08

    * Assert reset. This is when the new data is captured into the PLL.
        Set APLL_CTRL[31:0] = 0x0000_2D09h: [BYPASS] = 0x1 & [RESET] = 0x1
        pm 0xfd1a0020 0x00002D09

    * 5. Deassert reset.
        Set APLL_CTRL[31:0] = 0x0000_2D08h: [BYPASS] = 0x1 [RESET] = 0x0
        pm 0xfd1a0020 0x00002D08
    * 6. Check for LOCK. Wait until: PLL_STATUS [APLL_LOCK] = 0x1
            while (dm 0xfd1a0044 != 0x1) do wait // Pseudo code
    * 7. Deassert bypass.
            Set APLL_CTRL[31:0] = 0x0000_2D00h: [BYPASS] = 0x00
            pm 0xfd1a0020 0x00002D00
    */

    apll_ctrl_reg = ps_ref_ctrl_reg + (APLL_CTRL_OFFSET>>2);
    apll_cfg_reg = ps_ref_ctrl_reg + (APLL_CFG_OFFSET>>2);
    apll_status_reg = ps_ref_ctrl_reg + (APLL_STATUS_OFFSET>>2);

    /*
    ps_clk_vals array structure
    {
        45, //        fbdiv = ,
        0, //        div = ,

        3, //        cp = ,
        12, //        res = ,
        3, //        lfhf = ,
        63, //        lockdly = ,
        825, //        lockcnt = ,
    },
    */

    /* step 1
    * Set APLL_CTRL = 0x0000_2D00: [DIV2] = 0x0, [FBDIV] = 0X2D
            pm 0xfd1a0020 0x00002D00
    */

    unsigned int* curr_clk_str; 
    curr_clk_str = &ps_clk_vals[ind][0];

    *apll_ctrl_reg = (curr_clk_str[IND_DIV] << 16)
                       | (curr_clk_str[IND_FBDIV] << 8);
                        


    /* step 2
        * Program the control data for APLL_CFG using the data in Table 1.
                Set APLL_CFG[31:25] = 0x3F // LOCK_DLY
                Set APLL_CFG[22:13] = 0x339 // LOCK_CNT
                Set APLL_CFG[11:10] = 0x3 // LFHF
                Set APLL_CFG[8:5] = 0x3 // CP
                Set APLL_CFG[3:0] = 0x12 // RES

        * Set APLL_CFG[31:0] = 0x7E67_2C6C
                pm 0xfd1a0024 0x7E672C6C

        * Program the bypass:
                Set APLL_CTRL[31:0] = 0x0000_2D08h: [BYPASS] = 0x1
                pm 0xfd1a0020 0x00002D08

    */
    
    *apll_cfg_reg = (curr_clk_str[IND_LOCKDLY] << 25)       // delay
                     | (curr_clk_str[IND_LOCKCNT] <<13)       // count  
                     | (curr_clk_str[IND_LFHF] <<10)       //lfhf
                     | (curr_clk_str[IND_CP] <<5 )       // cp
                     | (curr_clk_str[IND_RES] )      ;     // res

    /* Step 3:
    Program the bypass:
    Set APLL_CTRL[31:0] = 0x0000_2D08h: [BYPASS] = 0x1
    pm 0xfd1a0020 0x00002D08
    */

    *apll_ctrl_reg = (*apll_ctrl_reg) | 0x8; 

    /* step 4
    * Assert reset. This is when the new data is captured into the PLL.
        Set APLL_CTRL[31:0] = 0x0000_2D09h: [BYPASS] = 0x1 & [RESET] = 0x1
        pm 0xfd1a0020 0x00002D09
    */

    *apll_ctrl_reg = (*apll_ctrl_reg) | 0x1; 

    /* step 5
    * 5. Deassert reset.
        Set APLL_CTRL[31:0] = 0x0000_2D08h: [BYPASS] = 0x1 [RESET] = 0x0
        pm 0xfd1a0020 0x00002D08
    */

    *apll_ctrl_reg = (*apll_ctrl_reg) & (0xFFFFFFFE); 

    /*
    * 6. Check for LOCK. Wait until: PLL_STATUS [APLL_LOCK] = 0x1
            while (dm 0xfd1a0044 != 0x1) do wait // Pseudo code
    */

    
    while((*apll_status_reg) & 1 == 0){
        ;
    }

    /*
    * 7. Deassert bypass.
            Set APLL_CTRL[31:0] = 0x0000_2D00h: [BYPASS] = 0x00
            pm 0xfd1a0020 0x00002D00
    */
    
    *apll_ctrl_reg = (*apll_ctrl_reg) & (0xFFFFFFF7); 

    return;
}

void set_random_pl_clk(int ind){
    int clk = pl_clk_divs[ind] ;
    //printf("clk = %d" ,clk);

    unsigned int value = (1<<24) // bit 24 enables clock
     | (1<<16) // bit 23:16 is divisor 1
      | (clk <<8); // bit 15:0 is clock divisor 0
      // frequency = 1.5Ghz/divisor0/divisor1
      // example = 1.5Ghz/6=250MHz


    //printf("0x%.8x" , pl_ref_ctrl_reg);
    //printf("0x%.8x" , value);

    unsigned int *pl0 = pl_ref_ctrl_reg;
    pl0 += 0xC0;

    *pl0 = value;

    DEBUG_PRINT("Pl clk address,val %p = 0x%.8x \n" , pl0, value);
    
    return;
}

void unmap_regions(){

    DEBUG_PRINT("unmapping memory\n");

    DEBUG_PRINT("unmapping OCM\n");
    munmap(ocm,4096);

    DEBUG_PRINT("unmapping OCM_BACK\n");
    munmap(ocm_back,4096);

    DEBUG_PRINT("unmapping CDMA\n");
    munmap(cdma_virtual_address,4096);

    DEBUG_PRINT("unmapping PL ref\n");
    munmap(pl_ref_ctrl_reg,4096);

    DEBUG_PRINT("unmapping PS ref\n");
    munmap(ps_ref_ctrl_reg,4096);
    
    DEBUG_PRINT("unmapping BRAM ref\n");
    munmap(BRAM_virtual_address,4096);

    return;
}

// Handler for SIGINT, caused by
// Ctrl-C at keyboard
void handle_sigint(int sig)
{

    printf("Caught signal %d\n", sig);
    printf("Test passed: %d loops of %d 32-bit words!!\n", num_loops,
            num_words); 
    unmap_regions();

    exit(0);
}


void initilize(){

    DEBUG_PRINT("Opening /dev/mem\n");
    dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    
    if(dh == -1)
        printf("Error openig the file");
    else
        DEBUG_PRINT("Successful opening /dev/mem\n");

    cdma_virtual_address = mmap(NULL, 
                                4096, 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, 
                                dh, 
                                CDMA); // Memory map AXI Lite register block

    if ( cdma_virtual_address == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 
    }
    else {
        DEBUG_PRINT("CMDA mmap successful\n");
    }


    BRAM_virtual_address = mmap(NULL, 
                                4096, 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, 
                                dh, 
                                BRAM); // Memory map AXI Lite register block

    if ( BRAM_virtual_address == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 
    }
    else {
        DEBUG_PRINT("BRAM mmap successful\n");
    }

    pl_ref_ctrl_reg = mmap(NULL,
                            0x1000,
                            PROT_READ|PROT_WRITE,
                            MAP_SHARED, 
                            dh, 
                            PL_CTRL_REG );

    if ( pl_ref_ctrl_reg == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 
    }
    else {
        DEBUG_PRINT("PL ref  mmap successful\n");
    }


    ps_ref_ctrl_reg = mmap(NULL,
                                0x1000,
                                PROT_READ|PROT_WRITE,
                                MAP_SHARED, 
                                dh, 
                                PS_CTRL_REG );

    if ( ps_ref_ctrl_reg == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 	// lazy usage
    }
    else {
        DEBUG_PRINT("PS ref  mmap successful\n");
    }


    //printf("memory allocation\n");

    ocm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dh, OCM);

    if ( ocm == MAP_FAILED ){
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 
    }
    else {
        DEBUG_PRINT("OCM mmap successful\n");
    }

    ocm_back = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dh,
                        OCM_BACK);

    if ( ocm_back == MAP_FAILED ){
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        handle_sigint(0); 
    }
    else {
        DEBUG_PRINT("OCM_BACK mmap successful\n");
    }

    return;
}

int evaluate(unsigned int * mem1, 
                unsigned int * mem2, int size){

    DEBUG_PRINT("Evaluating %p -- %p \n", mem1, mem2);

    //compare the values in bram with expected values 
    for(int i=0; i<size; i++)
    {
        if(*(mem1 + i) != *(mem2 + i)) {
           DEBUG_PRINT("RAM result: 0x%.8x and c result is 0x%.8x  element %d\n", 
                                *(mem1 + i), *(mem2 + i), i);

            DEBUG_PRINT("test failed!!\n");
            //DEBUG_PRINT("Mem1\n");
            //memdump(mem1, 4096);
            //DEBUG_PRINT("Mem2\n");
            //memdump(mem2, 4096);

            unmap_regions();

            return -1;
        }
    }
    DEBUG_PRINT("Passed \n");

    return 0;
}

int flags[1024] ; 

void fill_ocm_with_random(){
    DEBUG_PRINT("Filling arr with random values\n");

    //int flags[1024] ; 
    int num_filled = 0;

    for(int i = 0; i< 1024 ; i++){
        flags[i] = 0;
        ocm[i] =0; //reset ocm
    }

    while(num_filled < 1024){
	    value = rand();                 // Write random data
        offset = (rand() & MAP_MASK) >> 2;

        if(!flags[offset]){
            ocm[offset] = value;
            flags[offset] = 1;
            num_filled++;
        }
        
    }

    DEBUG_PRINT("Completed filling ARR with random values\n");
    return ;
}


void test1(){
    printf("%s\n", __func__);

    num_loops = 0;
    num_words = 0;

	srand(time(0));         // Seed the ramdom number generator        
    DEBUG_PRINT("Random number set\n");
	                            		

    // RESET DMA
    // what does it mean to reset DMA
    //dma_set(cdma_virtual_address, CDMACR, 0x04);

    while (1) {
       
        num_loops++;
        for(int clk1 = 0; clk1 < 5; clk1++){
            //DEBUG_PRINT("Setting PS clock freq = %f\n", ps_clks[clk1]);
            //set_random_ps_clk(clk1);

            for(int clk2 = 0; clk2 < 5; clk2++){

                DEBUG_PRINT("Setting PL clock freq = %f\n", pl_clks[clk2]);
                set_random_pl_clk(clk2);

                fill_ocm_with_random();

                //address = regs + (((target_addr + offset) & MAP_MASK)>>2);   
                for(int offset = 0; offset < 1024; offset++){
                    //address = BRAM_virtual_address + ((offset & MAP_MASK) >> 2);

                    address = BRAM_virtual_address + offset;
                    *address = ocm[offset]; 			    // perform write command

                    //DEBUG_PRINT("0x%.8x" , (offset));
                    //DEBUG_PRINT(" = 0x%.8x\n", *address);// display register value
                }
                
                num_words += 1024;

                evaluate(ocm, BRAM_virtual_address, 1024);
            }
        }
    }

    return;
}

void test2(){

    printf("%s\n", __func__);
    num_loops = 0;
    num_words = 0;

    while (1) {

        num_loops++; 

        dma_set(cdma_virtual_address, CDMACR, 0x04);

        for(int clk1 = 0; clk1 < 5; clk1++){
            //DEBUG_PRINT("Setting PS clock freq = %f\n", ps_clks[clk1]);
            //set_random_ps_clk(clk1);

            for(int clk2 = 0; clk2 < 5; clk2++){

                //set_random_pl_clk(clk2);

                fill_ocm_with_random(ocm);

                //transfer(cdma_virtual_address, 1024);
                transfer_dma(cdma_virtual_address,1024,OCM, BRAM_DMA);

                num_words += 1024;

                if( evaluate(ocm, BRAM_virtual_address, 1024) < 0)
                    exit(1);

            }
        }
    }

    return;

}

void test3(){

    printf("%s\n", __func__);
    num_loops = 0;
    num_words = 0;

    while (1) {
        num_loops++;

        dma_set(cdma_virtual_address, CDMACR, 0x04);

        for(int clk1 = 0; clk1 < 5; clk1++){
            //DEBUG_PRINT("Setting PS clock freq = %f\n", ps_clks[clk1]);
            //set_random_ps_clk(clk1);

            for(int clk2 = 0; clk2 < 5; clk2++){
                //set_random_ps_clk(clk2);

                set_random_pl_clk(clk2);

                fill_ocm_with_random();

                //dma_set(cdma_virtual_address, CDMACR, 0x04);

                transfer_dma(cdma_virtual_address,1024,OCM, BRAM_DMA);

                //dma_set(cdma_virtual_address, CDMACR, 0x04);

                transfer_dma(cdma_virtual_address,1024,BRAM_DMA,OCM_BACK);

                num_words += 1024;

                if( evaluate(ocm, ocm_back, 1024) < 0)
                    exit(1);
            }
        }
    }

    return;
}


int main() {

#ifndef TEST1
    test_number = 1;
#elif TEST2
    test_number = 2;
#else
    test_number = 2;
#endif

    DEBUG_PRINT("Using test number = %d\n", test_number);

    initilize();

    signal(SIGINT, handle_sigint);
	srand(time(0));         // Seed the ramdom number generator        
	//srand(0);         // Seed the ramdom number generator        
	                            		
    if(test_number == 1){
        test1();
    }
    else if(test_number == 2) {
        test2();
    }
    else {
        test3();
    }

    return 0;
}

