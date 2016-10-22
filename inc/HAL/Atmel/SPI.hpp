#pragma once

#include "HAL/Atmel/Registers.hpp"

namespace HAL {
namespace Atmel {

using namespace HAL::Atmel::Registers;

enum class SPIPrescaler: uint8_t {
    _4, _16, _64, _128, _2, _8, _32
};

#if F_CPU == 16000000
constexpr auto SPI8MHz = SPIPrescaler::_2;
constexpr auto SPI4MHz = SPIPrescaler::_4;
constexpr auto SPI2MHz = SPIPrescaler::_8;
constexpr auto SPI1MHz = SPIPrescaler::_16;
constexpr auto SPI500KHz = SPIPrescaler::_32;
constexpr auto SPI250KHz = SPIPrescaler::_64;
constexpr auto SPI125KHz = SPIPrescaler::_128;
#endif

class SPIMaster {
public:
    inline SPIMaster() {
        enable();
    }

    inline ~SPIMaster() {
        disable();
    }

    void setClockPrescaler(SPIPrescaler p) const;

    /**
     * Enables SPI master by configuring the MOSI(11), MISO(12) and SCK(13) pins for
     * SPI operation, and by enabling hardware SPI master. Those pins can not be used
     * for general purpose I/O during SPI operation.
     *
     * Default settings are:
     *   MSBit first
     *   SCK low when idle
     *   sample on leading SCK edge
     *
     * Pin 10 is also configured as output, and must be left configured as output during
     * SPI operation. Any pin 0-10 can be used as slave select pin for the peripheral(s)
     * being controlled.
     *
     * The SPI SCK clock speed is set to clock/2.
     */
    void enable() const;

    /**
     * Disables SPI mode, releasing pins 10-13 to be used as general purpose I/O.
     */
    inline void disable() const {
        SPE.clear();
    }

    /**
     * Sends the given byte on MOSI, while reading a byte on MISO and returning that.
     * Blocks until the transfer is complete.
     */
    uint8_t transceive(uint8_t out) const;

    /**
     * Sends the given byte on MOSI, ignoring anything on MISO. Blocks until the transfer
     * is complete.
     */
    void send(uint8_t out) const;
};

}
}

