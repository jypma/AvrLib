#pragma once

#include <stdint.h>
#include "Option.hpp"

namespace Sensirion {

namespace Impl {

/**
 * Driver for SHT21 and SHT25. Based off https://github.com/misenso/SHT2x-Arduino-Library/ .
 * Max TWI frequency is 0.4 MHz, according to data sheet.
 */
template <typename twi_t>
class SHT {
	static_assert(twi_t::frequency <= 400000, "TWI frequency must be <0.4MHz according to datasheet.");

	static constexpr uint8_t address = 0x40;

	twi_t * const twi;

	Option<uint16_t> readSensor(uint8_t command)
	{
	    if (!twi->write(address, command)) {
	    	return none();
	    }

	    uint8_t msb, lsb, checksum;
	    if (twi->read(address, &msb, &lsb, &checksum)) {
	    	return (msb << 8) | (lsb & ~0x03); // clear two low bits (status bits)
	    } else {
	    	return none();
	    }
	}

public:
	SHT(twi_t &t): twi(&t) {}

	Option<uint16_t> getHumidity() {
		return readSensor(0xE5) * (1250.0 / 65536.0) - 6;
	}

	Option<int16_t> getTemperature() {
		return (readSensor(0xE3) * (175.72 / 65536.0) - 46.85) * 10;
	}
};

}

template <typename twi_t>
Impl::SHT<twi_t> SHT(twi_t &twi) {
	return { twi };
}

}


