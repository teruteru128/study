
#ifndef BIT_SET_H
#define BIT_SET_H

#define unitIndex(bitIndex) ((bitIndex) >> 6)
#define bit(bitIndex) (1UL << ((bitIndex) & ((1 << 6) - 1)))

#endif

