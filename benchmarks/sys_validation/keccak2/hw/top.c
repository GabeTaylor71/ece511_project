#include "hw_defines.h"

void top(uint64_t in_state_addr,
		 uint64_t out_state_addr) {

	//Define Device MMRs
	volatile uint8_t  * KECCAK2Flags  = (uint8_t *)KECCAK2;
	volatile uint8_t  * DmaFlags   = (uint8_t  *)(DMA_Flags);
	volatile uint64_t * DmaRdAddr  = (uint64_t *)(DMA_RdAddr);
	volatile uint64_t * DmaWrAddr  = (uint64_t *)(DMA_WrAddr);
	volatile uint32_t * DmaCopyLen = (uint32_t *)(DMA_CopyLen);

	//Transfer Input Matrices
	//Transfer IN_STATE
	*DmaRdAddr  = in_state_addr;
	*DmaWrAddr  = IN_STATE;
	*DmaCopyLen = state_size;
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);
	
	//Transfer OUT_STATE
	*DmaRdAddr  = OUT_STATE;
	*DmaWrAddr  = out_state_addr;
	*DmaCopyLen = state_size;
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);
	return;
}