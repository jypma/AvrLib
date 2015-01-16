#include "avr/common.h"

uint8_t sfr_mem[256];

void cli() {
    SREG &= ~_BV(SREG_I);
}

void sei() {
    SREG |= _BV(SREG_I);
}
