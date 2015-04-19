#ifndef PARITY_H
#define PARITY_H

#include <stdint.h>

inline uint8_t parity_even_bit(uint8_t b) {
    return __builtin_parity(b);
}

#endif
