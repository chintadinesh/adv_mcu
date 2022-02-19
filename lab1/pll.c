#include "stdio.h"
#include "stdlib.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



int main(int argc, char * argv[]) {
    int dh = open("/dev/mem", O_RDWR | O_SYNC);
    if(dh == -1) {
        printf("Unable to open /dev/mem\n");
    }
    uint32_t* clk_reg = mmap(NULL,
                            0x1000,
                            PROT_READ|PROT_WRITE,
                            MAP_SHARED, 
                            dh, 
                            0xFF5E0000);

    int i = 0;
    uint32_t* pl0 = clk_reg;

    pl0+=0xC0; // PL0_REF_CTRL reg offset 0xC0
    *pl0 = (1<<24) // bit 24 enables clock
                | (1<<16) // bit 23:16 is divisor 1
                | (6<<8); // bit 15:0 is clock divisor 0
    // frequency = 1.5Ghz/divisor0/divisor1
    // = 1.5Ghz/6=250MHz

    munmap(clk_reg, 0x1000);
    return 0;
}
