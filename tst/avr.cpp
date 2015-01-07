#include "avr/common.h"

volatile uint8_t sfr_io[256];

void cli() {
    SREG |= _BV(SREG_I);
}
