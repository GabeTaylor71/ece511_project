#include "hw_defines.h"


void KeccakF1600_StatePermute(void *state)
{
    volatile u8 * m1 = (u8 *)MATRIX;

    unsigned int round, x, y, j, t;
    uint8_t LFSRstate = 0x01;
    static tKeccakLane C[5], D;

    for(round=0; round<24; round++) {
        {   /* === θ step (see [Keccak Reference, Section 2.3.2]) === */
            

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