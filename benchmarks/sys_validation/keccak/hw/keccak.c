#include "hw_defines.h"

// CODE FOR BYTE ARRAY ACCESSES
void keccak() {

    uint8_t *state = (uint8_t*) STATE;
    state[0] = 0xF; 
    uint64_t *C = (uint64_t*)C_ARR; 
    uint64_t *D = (uint64_t*)D_ARR; 
    uint64_t current; 
    uint64_t* temp = (uint64_t*)TEMP_ARR;
    temp[0] = 0xf; 
    C[0] = 0xf; 
    unsigned int round, x, y, j, t, i;
    uint8_t LFSRstate = 0x01;

    for (int round = 0; round < 24; round++) { //TODO: CHANGE THIS 
        /* θ step */
        for (int x = 0; x < 5; x++) {
            C[x] = 0;
            for (int y = 0; y < 5; y++) {
                uint64_t u = 0;
                #pragma clang loop unroll_count(32)
                for (int i = 7; i >= 0; --i) {
                    u <<= 8;
                    u |= state[(x + 5 * y) * 8 + i];
                }
                C[x] ^= u;
            }
        }
        #pragma clang loop unroll_count(32)
        for (int x = 0; x < 5; x++) {
            D[i] = C[(x + 4) % 5] ^ ((((uint64_t)C[(x + 1) % 5]) << 1));
            #pragma clang loop unroll_count(32)
            for (y = 0; y < 5; y++) {
                uint64_t u = D[i];
                #pragma clang loop unroll_count(32)
                for (int i = 0; i < 8; ++i) {
                    state[(x + 5 * y) * sizeof(uint64_t) + i] ^= u;
                    u >>= 8;
                }
            }
        }

        /* ρ and π steps */
        x = 1; y = 0;
        uint64_t u = 0;
        #pragma clang loop unroll_count(32)
        for (int i = 7; i >= 0; --i) {
            u <<= 8;
            u |= state[(x + 5 * y) * sizeof(uint64_t) + i];
        }
        current = u;
        #pragma clang loop unroll_count(32)
        for (int t = 0; t < 24; t++) {
            unsigned int r = ((t + 1) * (t + 2) / 2) % 64;
            unsigned int Y = (2 * x + 3 * y) % 5; x = y; y = Y;
            u = 0;
            #pragma clang loop unroll_count(32)
            for (int i = 7; i >= 0; --i) {
                u <<= 8;
                u |= state[(x + 5 * y) * sizeof(uint64_t) + i];
            }
            uint64_t temp_val = u;
            u = (((uint64_t)current) << r);
            #pragma clang loop unroll_count(32)
            for (int i = 0; i < 8; ++i) {
                state[(x + 5 * y) * sizeof(uint64_t) + i] = u;
                u >>= 8;
            }
            current = temp_val;
        }

        /* χ step */
        #pragma clang loop unroll_count(32)
        for (y = 0; y < 5; y++) {
            #pragma clang loop unroll_count(32)
            for (int x = 0; x < 5; x++) {
                u = 0;
                #pragma clang loop unroll_count(32)
                for (int i = 7; i >= 0; --i) {
                    u <<= 8;
                    u |= state[(x + 5 * y) * sizeof(uint64_t) + i];
                }
                temp[x] = u;
            }
            #pragma clang loop unroll_count(32)
            for (int x = 0; x < 5; x++) {
                u = temp[x] ^ ((~temp[(x + 1) % 5]) & temp[(x + 2) % 5]);
                #pragma clang loop unroll_count(32)
                for (int i = 0; i < 8; ++i) {
                    state[(x + 5 * y) * sizeof(uint64_t) + i] = u;
                    u >>= 8;
                }
            }
        }

        /* ι step */
        #pragma clang loop unroll_count(32)
        for (j = 0; j < 7; j++) {
            unsigned int bitPosition = (1 << j) - 1;
            int result = (LFSRstate & 0x01) != 0;
            if ((LFSRstate & 0x80) != 0)
                LFSRstate = (LFSRstate << 1) ^ 0x71;
            else
                LFSRstate <<= 1;
            if (result) {
                u = (uint64_t)1 << bitPosition;
                #pragma clang loop unroll_count(32)
                for (int i = 0; i < 8; ++i) {
                    state[i] ^= u;
                    u >>= 8;
                }
            }
        }
    }
}


// CODE FOR 64 BIT ACCESS PATTERN
// static void Keccak() {
//     uint64_t (*A)[5] = (uint64_t (*)[5]) STATE; // Treat state as 5x5 matrix of uint64_t
//     uint64_t C[5], D, temp[5];
//     unsigned int round, x, y, j, t;
//     uint8_t LFSRstate = 0x01;

//     for (round = 0; round < 24; round++) {
//         /* θ step */
//         for (x = 0; x < 5; x++) {
//             C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
//         }
//         for (x = 0; x < 5; x++) {
//             D = C[(x + 4) % 5] ^ ((((uint64_t)C[(x + 1) % 5]) << 1) ^ (((uint64_t)C[(x + 1) % 5]) >> (64 - 1)));
//             for (y = 0; y < 5; y++) {
//                 A[x][y] ^= D;
//             }
//         }

//         /* ρ and π steps */
//         x = 1; y = 0;
//         uint64_t current = A[x][y];
//         for (int t = 0; t < 24; t++) {
//             unsigned int r = ((t + 1) * (t + 2) / 2) % 64;
//             unsigned int Y = (2 * x + 3 * y) % 5; x = y; y = Y;
//             uint64_t temp_val = A[x][y];
//             A[x][y] = (((uint64_t)current) << r) ^ (((uint64_t)current) >> (64 - r));
//             current = temp_val;
//         }

//         /* χ step */
//         for (y = 0; y < 5; y++) {
//             for (x = 0; x < 5; x++) {
//                 temp[x] = A[x][y];
//             }
//             for (x = 0; x < 5; x++) {
//                 A[x][y] = temp[x] ^ ((~temp[(x + 1) % 5]) & temp[(x + 2) % 5]);
//             }
//         }

//         /* ι step */
//         for (j = 0; j < 7; j++) {
//             unsigned int bitPosition = (1 << j) - 1;
//             int result = (LFSRstate & 0x01) != 0;
//             if ((LFSRstate & 0x80) != 0)
//                 LFSRstate = (LFSRstate << 1) ^ 0x71;
//             else
//                 LFSRstate <<= 1;
//             if (result) {
//                 A[0][0] ^= (uint64_t)1 << bitPosition;
//             }
//         }
//     }
// }