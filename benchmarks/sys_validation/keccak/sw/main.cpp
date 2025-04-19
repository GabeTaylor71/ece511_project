#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "bench.h"
#include "../../../common/m5ops.h"
#include "../keccak_clstr_hw_defines.h"
#define FOR(i,n) for(i=0; i<n; ++i)
typedef unsigned char u8;
typedef unsigned long long int u64;
typedef unsigned int ui;

int LFSR86540(u8 *R) { (*R)=((*R)<<1)^(((*R)&0x80)?0x71:0); return ((*R)&2)>>1; }
#define ROL(a,o) ((((u64)a)<<o)^(((u64)a)>>(64-o)))
static u64 load64(const u8 *x) { ui i; u64 u=0; FOR(i,8) { u<<=8; u|=x[7-i]; } return u; }
static void store64(u8 *x, u64 u) { ui i; FOR(i,8) { x[i]=u; u>>=8; } }
static void xor64(u8 *x, u64 u) { ui i; FOR(i,8) { x[i]^=u; u>>=8; } }
#define rL(x,y) load64((u8*)s+8*(x+5*y))
#define wL(x,y,l) store64((u8*)s+8*(x+5*y),l)
#define XL(x,y,l) xor64((u8*)s+8*(x+5*y),l)

keccak_struct kec;

volatile uint8_t  * top   = (uint8_t  *)(TOP + 0x00);
volatile uint32_t * val_to = (uint32_t *)(TOP + 0x01);
volatile uint32_t * val_from = (uint32_t *)(TOP + 0x09);

int __attribute__ ((optimize("0"))) main(void) {
	m5_reset_stats();
    uint32_t base = 0x80c00000;
	TYPE *matrix_a = (TYPE *)base;
	TYPE *matrix_b = (TYPE *)(base+8*5*5);
	TYPE *check = (TYPE *)(base+16*5*5);
	int row_size = 5;
    int col_size = 5;
    volatile int count = 0;
	stage = 0;

    kec.matrix_to = matrix_a;
    kec.matrix_from = matrix_b;
    kec.row_size = row_size;
    kec.col_size = col_size;

    printf("Generating data\n");
    genData(&kec);
    printf("Data generated\n");

    *val_matrix_a = (uint32_t)(void *)matrix_a;
    *val_matrix_b = (uint32_t)(void *)matrix_b;
    // printf("%d\n", *top);
    *top = 0x01;
    while (stage < 1) count++;

    printf("Job complete\n");
#ifdef CHECK
    printf("Checking result\n");
    printf("Running bench on CPU\n");
	bool fail = false;

    ui r,x,y,i,j,Y; u8 R=0x01; u64 C[5],D;
    for(i=0; i<24; i++) {
        /*θ*/ FOR(x,5) C[x]=rL(x,0)^rL(x,1)^rL(x,2)^rL(x,3)^rL(x,4); FOR(x,5) { D=C[(x+4)%5]^ROL(C[(x+1)%5],1); FOR(y,5) XL(x,y,D); }
        /*ρπ*/ x=1; y=r=0; D=rL(x,y); FOR(j,24) { r+=j+1; Y=(2*x+3*y)%5; x=y; y=Y; C[0]=rL(x,y); wL(x,y,ROL(D,r%64)); D=C[0]; }
        /*χ*/ FOR(y,5) { FOR(x,5) C[x]=rL(x,y); FOR(x,5) wL(x,y,C[x]^((~C[(x+1)%5])&C[(x+2)%5])); }
        /*ι*/ FOR(j,7) if (LFSR86540(&R)) XL(0,0,(u64)1<<((1<<j)-1));
    }

    printf("Comparing CPU run to accelerated run\n");
    for(i=0; i<ROW*COL; i++) {
        if(matrix_b[i] != check[i]) {
            printf("Expected:%f Actual:%f\n", check[i], matrix_b[i]);
            fail = true;
            break;
        }
    }
    if(fail)
        printf("Check Failed\n");
    else
        printf("Check Passed\n");
#endif
	m5_dump_stats();
	m5_exit();
}
