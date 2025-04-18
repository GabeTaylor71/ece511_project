#include "../defines.h"
volatile int stage;

typedef struct {
    TYPE * matrix_to;
    TYPE * matrix_from;
    int row_size;
    int col_size;
} keccak_struct;

void genData(keccak_struct * kec) {
    int r, c, k, mult, sum;

    for( r=0; r < kec->row_size; r++ ) {
        for( c=0; c < kec->col_size; c++ ) {
            kec->matrix_to[r * kec->col_size + c] = 0;
        }
    }
}


