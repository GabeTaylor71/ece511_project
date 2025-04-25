#include "hw_defines.h"


void KeccakF1600()
{
    long int *m1 = (long int *)MATRIX;
    ui r, x, y, i, j, Y;
    u8 R = 0x01;
    u64 D;

    u64* C = (u64*) TEMP;

    // printf("sth");

    for (i = 0; i < 24; i++) {
        //("iterating\n");
        // θ step
        for (x = 0; x < 5; x++) {
            C[x] = 0;
            for (y = 0; y < 5; y++) {
                u8 *ptr = (u8*)m1 + 8 * (x + 5 * y);
                u64 u = 0;
                for (ui k = 0; k < 8; k++) {
                    u <<= 8;
                    u |= ptr[7 - k];
                }
                C[x] ^= u;
            }
        }
        for (x = 0; x < 5; x++) {
            D = C[(x + 4) % 5] ^ (((u64)C[(x + 1) % 5]));
            for (y = 0; y < 5; y++) {
                u8 *ptr = (u8*)m1 + 8 * (x + 5 * y);
                u64 u = 0;
                for (ui k = 0; k < 8; k++) {
                    u <<= 8;
                    u |= ptr[7 - k];
                }
                u ^= D;
                for (ui k = 0; k < 8; k++) {
                    ptr[k] = u >> (8 * k);
                }
            }
        }

        // ρ and π steps
        x = 1; y = r = 0;
        {
            u8 *ptr = (u8*)m1 + 8 * (x + 5 * y);
            u64 Dload = 0;
            for (ui k = 0; k < 8; k++) {
                Dload <<= 8;
                Dload |= ptr[7 - k];
            }

            for (j = 0; j < 24; j++) {
                r += j + 1;
                Y = (2 * x + 3 * y) % 5;
                x = y;
                y = Y;
                u8 *ptr2 = (u8*)m1 + 8 * (x + 5 * y);
                u64 Ctmp = 0;
                for (ui k = 0; k < 8; k++) {
                    Ctmp <<= 8;
                    Ctmp |= ptr2[7 - k];
                }
                u64 Rolled = ((u64)Dload << (r % 64)) ^ ((u64)Dload >> (64 - (r % 64)));
                for (ui k = 0; k < 8; k++) {
                    ptr2[k] = Rolled >> (8 * k);
                }
                Dload = Ctmp;
            }
        }

        // χ step
        for (y = 0; y < 5; y++) {
            for (x = 0; x < 5; x++) {
                u8 *ptr = (u8*)m1 + 8 * (x + 5 * y);
                u64 u = 0;
                for (ui k = 0; k < 8; k++) {
                    u <<= 8;
                    u |= ptr[7 - k];
                }
                C[x] = u;
            }
            for (x = 0; x < 5; x++) {
                u64 u = C[x] ^ ((~C[(x + 1) % 5]) & C[(x + 2) % 5]);
                u8 *ptr = (u8*)m1 + 8 * (x + 5 * y);
                for (ui k = 0; k < 8; k++) {
                    ptr[k] = u >> (8 * k);
                }
            }
        }

        // ι step
        for (j = 0; j < 7; j++) {
            u8 feedback = (R & 0x80) ? 0x71 : 0;
            R = (R << 1) ^ feedback;
            if ((R & 2) >> 1) {
                u8 *ptr = (u8*)m1 + 8 * (0 + 5 * 0);
                u64 u = 0;
                for (ui k = 0; k < 8; k++) {
                    u <<= 8;
                    u |= ptr[7 - k];
                }
                u ^= ((u64)1 << ((1 << j) - 1));
                for (ui k = 0; k < 8; k++) {
                    ptr[k] = u >> (8 * k);
                }
            }
        }
    }
    //m5_print("End\n");
}