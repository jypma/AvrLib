#include "avr/common.h"
#include "util/crc16.h"

uint8_t sfr_mem[256];

void cli() {
    SREG &= ~_BV(SREG_I);
}

void sei() {
    SREG |= _BV(SREG_I);
}

uint16_t _crc16_update(uint16_t crc, uint8_t a) {
    int i;
    crc ^= a;
    for (i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}
