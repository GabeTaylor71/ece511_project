#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "bench.h"
#include "../../../common/m5ops.h"
#include "../keccak_cluster_hw_defines.h"
#include "../defines.h"
#include <string.h>
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define readLane(x, y)          (((tKeccakLane*)state)[i(x, y)])
#define writeLane(x, y, lane)   (((tKeccakLane*)state)[i(x, y)]) = (lane)
#define XORLane(x, y, lane)     (((tKeccakLane*)state)[i(x, y)]) ^= (lane)
#define ROL64(a, offset) ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64-offset)))
#define i(x, y) ((x)+5*(y))

keccak_struct kec;
#include <stdint.h>
typedef uint64_t tKeccakLane;
static uint64_t load64(const uint8_t *x)
{
    int i;
    uint64_t u=0;

    for(i=7; i>=0; --i) {
        u <<= 8;
        u |= x[i];
    }
    return u;
}

/** Function to store a 64-bit value using the little-endian (LE) convention.
  * On a LE platform, this could be greatly simplified using a cast.
  */
static void store64(uint8_t *x, uint64_t u)
{
    unsigned int i;

    for(i=0; i<8; ++i) {
        x[i] = u;
        u >>= 8;
    }
}

/** Function to XOR into a 64-bit value using the little-endian (LE) convention.
  * On a LE platform, this could be greatly simplified using a cast.
  */
static void xor64(uint8_t *x, uint64_t u)
{
    unsigned int i;

    for(i=0; i<8; ++i) {
        x[i] ^= u;
        u >>= 8;
    }
}
/**
  * Function that computes the linear feedback shift register (LFSR) used to
  * define the round constants (see [Keccak Reference, Section 1.2]).
  */
  int LFSR86540(uint8_t *LFSR)
  {
      int result = ((*LFSR) & 0x01) != 0;
      if (((*LFSR) & 0x80) != 0)
          /* Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1 */
          (*LFSR) = ((*LFSR) << 1) ^ 0x71;
      else
          (*LFSR) <<= 1;
      return result;
  }

/**
 * Function that computes the Keccak-f[1600] permutation on the given state.
 */
 void KeccakF1600_StatePermute(void *state)
 {
     unsigned int round, x, y, j, t;
     uint8_t LFSRstate = 0x01;
 
     for(round=0; round<24; round++) {
         {   /* === θ step (see [Keccak Reference, Section 2.3.2]) === */
             tKeccakLane C[5], D;
 
             /* Compute the parity of the columns */
             for(x=0; x<5; x++)
                 C[x] = readLane(x, 0) ^ readLane(x, 1) ^ readLane(x, 2) ^ readLane(x, 3) ^ readLane(x, 4);
             for(x=0; x<5; x++) {
                 /* Compute the θ effect for a given column */
                 D = C[(x+4)%5] ^ ROL64(C[(x+1)%5], 1);
                 /* Add the θ effect to the whole column */
                 for (y=0; y<5; y++)
                     XORLane(x, y, D);
             }
         }
 
         {   /* === ρ and π steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) === */
             tKeccakLane current, temp;
             /* Start at coordinates (1 0) */
             x = 1; y = 0;
             current = readLane(x, y);
             /* Iterate over ((0 1)(2 3))^t * (1 0) for 0 ≤ t ≤ 23 */
             for(t=0; t<24; t++) {
                 /* Compute the rotation constant r = (t+1)(t+2)/2 */
                 unsigned int r = ((t+1)*(t+2)/2)%64;
                 /* Compute ((0 1)(2 3)) * (x y) */
                 unsigned int Y = (2*x+3*y)%5; x = y; y = Y;
                 /* Swap current and state(x,y), and rotate */
                 temp = readLane(x, y);
                 writeLane(x, y, ROL64(current, r));
                 current = temp;
             }
         }
 
         {   /* === χ step (see [Keccak Reference, Section 2.3.1]) === */
             tKeccakLane temp[5];
             for(y=0; y<5; y++) {
                 /* Take a copy of the plane */
                 for(x=0; x<5; x++)
                     temp[x] = readLane(x, y);
                 /* Compute χ on the plane */
                 for(x=0; x<5; x++)
                     writeLane(x, y, temp[x] ^((~temp[(x+1)%5]) & temp[(x+2)%5]));
             }
         }
 
         {   /* === ι step (see [Keccak Reference, Section 2.3.5]) === */
             for(j=0; j<7; j++) {
                 unsigned int bitPosition = (1<<j)-1; /* 2^j-1 */
                 if (LFSR86540(&LFSRstate))
                     XORLane(0, 0, (tKeccakLane)1<<bitPosition);
             }
         }
     }
 }
 
 /*
 ================================================================
 A readable and compact implementation of the Keccak sponge functions
 that use the Keccak-f[1600] permutation.
 ================================================================
 */
 
 #include <string.h>
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
 
 void Keccak(unsigned int rate, unsigned int capacity, const unsigned char *input, unsigned long long int inputByteLen, unsigned char delimitedSuffix, unsigned char *output, unsigned long long int outputByteLen)
 {
     uint8_t state[200];
     unsigned int rateInBytes = rate/8;
     unsigned int blockSize = 0;
     unsigned int i;
 
     if (((rate + capacity) != 1600) || ((rate % 8) != 0))
         return;
 
     /* === Initialize the state === */
     memset(state, 0, sizeof(state));
 
     /* === Absorb all the input blocks === */
     while(inputByteLen > 0) {
         blockSize = MIN(inputByteLen, rateInBytes);
         for(i=0; i<blockSize; i++)
             state[i] ^= input[i];
         input += blockSize;
         inputByteLen -= blockSize;
 
         if (blockSize == rateInBytes) {
             KeccakF1600_StatePermute(state);
             blockSize = 0;
         }
     }
 
     /* === Do the padding and switch to the squeezing phase === */
     /* Absorb the last few bits and add the first bit of padding (which coincides with the delimiter in delimitedSuffix) */
     state[blockSize] ^= delimitedSuffix;
     /* If the first bit of padding is at position rate-1, we need a whole new block for the second bit of padding */
     if (((delimitedSuffix & 0x80) != 0) && (blockSize == (rateInBytes-1)))
         KeccakF1600_StatePermute(state);
     /* Add the second bit of padding */
     state[rateInBytes-1] ^= 0x80;
     /* Switch to the squeezing phase */
     KeccakF1600_StatePermute(state);
 
     /* === Squeeze out all the output blocks === */
     while(outputByteLen > 0) {
         blockSize = MIN(outputByteLen, rateInBytes);
         memcpy(output, state, blockSize);
         output += blockSize;
         outputByteLen -= blockSize;
 
         if (outputByteLen > 0)
             KeccakF1600_StatePermute(state);
     }
 }

void FIPS202_SHA3_256(const unsigned char *input, unsigned int inputByteLen, unsigned char *output)
{
    Keccak(1088, 512, input, inputByteLen, 0x06, output, 32);
}

volatile uint8_t  * top   = (uint8_t  *)(TOP + 0x00);
volatile uint32_t * val_matrix = (uint32_t *)(TOP + 0x01);

int __attribute__ ((optimize("0"))) main(void) {
    uint32_t base = 0x80c00000;
	uint8_t *input = (uint8_t *)((char*)base);
    uint8_t *output = (uint8_t *)((char*)base + 1024);

    //FIPS202_SHA3_256(input, 1024, output);
    volatile int count = 0;
	stage = 0;
    kec.matrix = input;

    printf("Generating data\n");
    genData(&kec);
    printf("Data generated\n");
    
    //algo
    unsigned int rate = 1088;
    unsigned int capacity = 512;
    unsigned long long int inputByteLen = 1024;
    unsigned char delimitedSuffix = 0x06;
    unsigned long long int outputByteLen = 32;

    uint8_t state[200];
    unsigned int rateInBytes = rate/8;
    unsigned int blockSize = 0;
    unsigned int i;
    /* === Initialize the state === */
    
    memset(state, 0, sizeof(state));
	m5_reset_stats();
    /* === Absorb all the input blocks === */
    while(inputByteLen > 0) {
        blockSize = MIN(inputByteLen, rateInBytes);
        for(i=0; i<blockSize; i++)
            state[i] ^= input[i];
        input += blockSize;
        inputByteLen -= blockSize;

        if (blockSize == rateInBytes) {
            count = 0;
	        stage = 0;
            *val_matrix = (uint32_t)(void *)state;
            *top = 0x01;
            while (stage < 1) count++;
            printf("Job complete 1\n");
            blockSize = 0;
        }
    }

    /* === Do the padding and switch to the squeezing phase === */
    /* Absorb the last few bits and add the first bit of padding (which coincides with the delimiter in delimitedSuffix) */
    state[blockSize] ^= delimitedSuffix;
    /* If the first bit of padding is at position rate-1, we need a whole new block for the second bit of padding */
    if (((delimitedSuffix & 0x80) != 0) && (blockSize == (rateInBytes-1))){
        count = 0;
	    stage = 0;
        *val_matrix = (uint32_t)(void *)state;
        *top = 0x01;
        while (stage < 1) count++;
        printf("Job complete 2\n");
    }
    
    /* Add the second bit of padding */
    state[rateInBytes-1] ^= 0x80;
    count = 0;
	stage = 0;
    /* Switch to the squeezing phase */
    *val_matrix = (uint32_t)(void *)state;
    *top = 0x01;
    while (stage < 1) count++;
    printf("Job complete 3\n");

    /* === Squeeze out all the output blocks === */
    while(outputByteLen > 0) {
        blockSize = MIN(outputByteLen, rateInBytes);
        memcpy(output, state, blockSize);
        output += blockSize;
        outputByteLen -= blockSize;

        if (outputByteLen > 0){
            count = 0;
	        stage = 0;
            *val_matrix = (uint32_t)(void *)state;
            *top = 0x01;
            while (stage < 1) count++;
            printf("Job complete 4\n");
        }
    }

    printf("Job complete 5\n");
	m5_dump_stats();
	m5_exit();
}