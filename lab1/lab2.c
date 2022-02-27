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

#define CDMA                0xB0000000
#define BRAM                0xA0028000
#define BRAM_DMA            0xB0028000
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

#define OCM_BACK            0xFFFC2000

volatile unsigned int *regs, *address ;

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

void transfer(unsigned int *cdma_virtual_address, int length)
{
    dma_set(cdma_virtual_address, DA, BRAM_DMA); // Write destination address
    dma_set(cdma_virtual_address, SA, OCM); // Write source address
    dma_set(cdma_virtual_address, BTT, length*4);
    cdma_sync(cdma_virtual_address);
}

void transfer_back(unsigned int *cdma_virtual_address, int length)
{
    dma_set(cdma_virtual_address, DA, OCM_BACK); // Write destination address
    dma_set(cdma_virtual_address, SA, BRAM_DMA); // Write source address
    dma_set(cdma_virtual_address, BTT, length*4);
    cdma_sync(cdma_virtual_address);
}

uint32_t c[1024];
void fill_random(){
    //uint32_t c[20] = {4,6,2,6,3,8,0,4,6,8,3,42,7,8,2,75,2,69,6,1};  // 

    for(int i= 0; i < 1024; i++)
        c[i] = rand();

    return;
}

int main() {
    int dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    uint32_t* cdma_virtual_address = mmap(NULL, 
                                          4096, 
                                          PROT_READ | PROT_WRITE, 
                                          MAP_SHARED, 
                                          dh, 
                                          CDMA); // Memory map AXI Lite register block
    uint32_t* BRAM_virtual_address = mmap(NULL, 
                                          4096 * 2, 
                                          PROT_READ | PROT_WRITE, 
                                          MAP_SHARED, 
                                          dh, 
                                          BRAM); // Memory map AXI Lite register block

    printf("memory allocation\n");
    uint32_t* ocm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, 
                        MAP_SHARED, dh, OCM);
    uint32_t* ocm_back = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED,
                            dh, OCM_BACK);
    //uint32_t* ocm_back = ocm + (0x2000>>4); 


    ///////////////////////////////////////////////////////////////////////////////
    /////////////////           TEST 1 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

        /*
        fill_random();

        // writing to memory
        for(int offset = 0; offset < 1024; offset++){

            address = BRAM_virtual_address + offset;
            *address = c[offset]; 			    // perform write command

            //DEBUG_PRINT("0x%.8x" , (offset));
            //DEBUG_PRINT(" = 0x%.8x\n", *address);// display register value
        }

        for(int i=0; i<1024; i++)
        {
            if(BRAM_virtual_address[i] != c[i])
            {
                printf("RAM result: %d and c result is %d  element %d\n", 
                            BRAM_virtual_address[i], c[i], i);

                printf("test failed!!\n");
                break;
            }
        }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////// END OF    TEST 1 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////
    /////////////////           TEST 2 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

        fill_random();

        // writing to memory
        for(int offset = 0; offset < 1024; offset++){

            address = ocm + offset;
            *address = c[offset]; 			    // perform write command

            //DEBUG_PRINT("0x%.8x" , (offset));
            //DEBUG_PRINT(" = 0x%.8x\n", *address);// display register value
        }

        for(int i=0; i<1024; i++)
        {
            if(ocm[i] != c[i])
            {
                printf("OCM result: %d and c result is %d  element %d\n", 
                            ocm[i], c[i], i);

                printf("test failed!!\n");
                break;
            }
        }


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////// END OF    TEST 2 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////// 
    ///////////////// TEST 3 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////// END OF    TEST 3 ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    */
    
    // RESET DMA
    dma_set(cdma_virtual_address, CDMACR, 0x04);
    struct timeval start, end;

    for(int t = 0; t < 2; t++){

    //    printf("Source memory block:      "); memdump(virtual_source_address, 32);
    //    printf("Destination memory block: "); memdump(virtual_destination_address, 32);

        transfer(cdma_virtual_address, 1024);

        //dma_set(cdma_virtual_address, CDMACR, 0x04);

        transfer_back(cdma_virtual_address, 1024);

        for(int i=0; i<1024; i++)
        {
            if(ocm[i] != ocm_back[i])
            {
                printf("RAM result: 0x%.8x and c result is 0x%.8x  element %d\n", ocm[i], 
                            ocm_back[i], i);
                printf("test failed!!\n");

                munmap(ocm,4096);
                munmap(ocm_back,4096);

                munmap(cdma_virtual_address,4096);
                munmap(BRAM_virtual_address,4096);
                return -1;
            }
        }
    }

    printf("test passed!!\n");
    //munmap(ocm,65536);
    munmap(ocm,4096);
    munmap(ocm_back,4096);

    munmap(cdma_virtual_address,4096);
    munmap(BRAM_virtual_address,4096);
    return 0;
}
