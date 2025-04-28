#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "bench.h"
#include "../../../common/m5ops.h"
#include "../keccak_cluster_hw_defines.h"

volatile uint8_t  * top   = (uint8_t  *)(TOP + 0x00);
volatile uint32_t * val_state = (uint32_t *)(TOP + 0x01);
volatile int accelerator_offload_cnts = 0; 

int __attribute__ ((optimize("0"))) main(void) {
    uint32_t base = 0x80c00000;
	uint8_t * state = (uint8_t*) base;
    uint8_t * input = (uint8_t*) ((char*) base + (200)); 
    uint8_t * output = (uint8_t*) ((char*) base + (200 + 1024)); 

    

    memset(input, 0, 1024); 

    m5_reset_stats();
    // CALL SHA3_256 FUNCTION on 1KB input
    // printf("before executing full keccak\n"); 

    // printf("entered full keccak\n"); 
    // full_keccak(input, 1024, 0x06, output, 32, 1088, 512); 


    unsigned long long inputByteLen = 1024; 
    uint8_t delimitedSuffix = 0x06; 
    long long outputByteLen = 32;
    unsigned int rate = 1088;
    unsigned int capacity = 512; 

    // uint8_t state[200];
    unsigned int rateInBytes = rate / 8;
    unsigned int blockSize = 0;
    unsigned int i;

    // if (((rate + capacity) != 1600) || ((rate % 8) != 0))
    // return 0;

    // memset(state, 0, sizeof(state));

    // while (inputByteLen > 0) {
    //     blockSize = (inputByteLen < rateInBytes) ? (unsigned int)inputByteLen : rateInBytes;
    //     for (i = 0; i < blockSize; i++)
    //         state[i] ^= input[i];
    //     input += blockSize;
    //     inputByteLen -= blockSize;

    //     if (blockSize == rateInBytes) {
    //         // activate accelerator
    //         *val_state = (uint32_t)(void *)state;
            
    //         accelerator_offload_cnts += 1;
    //         *top = 0x01;
    //         int count = 0; 
    //         while (stage < 1) count++;

    //         blockSize = 0;
    //     }
    // }

    // state[blockSize] ^= delimitedSuffix;
    // if (((delimitedSuffix & 0x80) != 0) && (blockSize == (rateInBytes - 1))) {
    //     // call permute function again 
    //     state[rateInBytes - 1] ^= 0x80;

    //     // activate accelerator
    //     *val_state = (uint32_t)(void *)state;
    
    //     accelerator_offload_cnts += 1;
    
    //     *top = 0x01;
    //     int count = 0; 
    //     while (stage < 1) count++;
    
    // }


    // // get the squeezed outputs
    // // GENERATE OUTPUTS
    // while (outputByteLen > 0) {
    //     blockSize = (outputByteLen < rateInBytes) ? (unsigned int)outputByteLen : rateInBytes;
    //     memcpy(output, state, blockSize);
    //     output += blockSize;
    //     outputByteLen -= blockSize;

    //     if (outputByteLen > 0) {
    //         // activate accelerator
    //         *val_state = (uint32_t)(void *)state;

    //         accelerator_offload_cnts += 1;

    //         *top = 0x01;
    //         int count = 0; 
    //         while (stage < 1) count++;

    //     }
    //     // KeccakF1600_StatePermute(state);
    // }

             *val_state = (uint32_t)(void *)state;

             accelerator_offload_cnts += 1;

             *top = 0x01;
             int count = 0; 
             while (stage < 1) count++;

    printf("Job complete\n");
    printf("Number of accelerator offloads: %d.\n", accelerator_offload_cnts); 
    m5_dump_stats();

	m5_exit();
}