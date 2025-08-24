#ifndef __RAND_LIB
#define __RAND_LIB

#include <gba.h>

static u16_t seed = 0x1abf;
u16_t my_rand() { seed = rand(seed); return seed; }

#endif

