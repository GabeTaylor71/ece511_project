volatile int stage;

typedef struct {
    long int * matrix;
    int row_size;
    int col_size;
} keccak_struct;

void genData(keccak_struct * kec) {
    int r, c, k, mult, sum;

    for( r=0; r < kec->row_size; r++ ) {
        for( c=0; c < kec->col_size; c++ ) {
            kec->matrix[r * kec->col_size + c] = 0;
        }
    }
}


