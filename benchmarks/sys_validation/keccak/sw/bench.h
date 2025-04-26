volatile int stage;

typedef struct {
    uint8_t *matrix;
} keccak_struct;

void genData(keccak_struct * kec) {
    for(int i=0; i < 1024; i++ ) {
        kec->matrix[i] = 0;
    }
}


