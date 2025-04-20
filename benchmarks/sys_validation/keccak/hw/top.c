#include "hw_defines.h"

void top(uint64_t state_to_accl_addr,
		 uint64_t state_back_accl_addr) {

	//Define Device MMRs
	volatile uint8_t  * keccakFlags  = (uint8_t *)(KECCAK);
	volatile uint8_t  * DmaFlags   = (uint8_t  *)(DMA_Flags);
	volatile uint64_t * DmaRdAddr  = (uint64_t *)(DMA_RdAddr);
	volatile uint64_t * DmaWrAddr  = (uint64_t *)(DMA_WrAddr);
	volatile uint32_t * DmaCopyLen = (uint32_t *)(DMA_CopyLen);

	//Transfer Input Matrices
	//Transfer M1
	*DmaRdAddr  = state_to_accl_addr;
	*DmaWrAddr  = MATRIX;
	*DmaCopyLen = 8*5*5;
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);

	//Start the accelerated function
	*keccakFlags = DEV_INIT;
	//Poll function for finish
	while ((*keccakFlags & DEV_INTR) != DEV_INTR);

	//Transfer M3
	*DmaRdAddr  = MATRIX;
	*DmaWrAddr  = state_back_accl_addr;
	*DmaCopyLen = 8*5*5;
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);
	return;
}