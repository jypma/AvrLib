#include "avr/common.h"
#include "util/crc16.h"
#include "avr/sleep.h"
#include "avr/io.h"
#include <iostream>

std::function<void()> onSleep_cpu = nullptr;

uint16_t SpecialFunctionRegister16::lastAddr = 0;
SpecialFunctionRegister sfr_mem[256];
SpecialFunctionRegister16 sfr_mem16[255];
uint8_t eeprom_contents[1024];

SpecialFunctionRegister::SpecialFunctionRegister() {
	//std::cout << "this=" << uint64_t(this) << " SPDR=" << uint64_t(&SPDR) << std::endl;
	if (this == &SPDR) {
		callback = [] {
			// SPI complete
			SPSR |= _BV(SPIF);
		};
	} else if (this == &UCSR0A) {
		callback = [this] {
			// TXC bit should always be 1, indicating serial tx complete
			value |= _BV(TXC0);
		};
	}
}

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
