//Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include "../defines.h"
#include "../keccak2_clstr_hw_defines.h"

#define DEV_INIT	0x01
#define DEV_INTR	0x04

//Specify row/column sizes
#define state_size	200