/*
 * SPI.hpp
 *
 *  Created on: Jan 16, 2015
 *      Author: jan
 */

#ifndef SPI_HPP_
#define SPI_HPP_

#include <avr/io.h>

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
    SPIMaster() {
        enable();
    }

    ~SPIMaster() {
        disable();
    }

    void setClockPrescaler(SPIPrescaler p) const {
        uint8_t bits = static_cast<uint8_t>(p);
        if (bits & 1) {
            SPCR |= _BV(SPR0);
        } else {
            SPCR &= ~_BV(SPR0);
        }
        if (bits & 2) {
            SPCR |= _BV(SPR1);
        } else {
            SPCR &= ~_BV(SPR1);
        }
        if (bits & 4) {
            SPSR |= _BV(SPI2X);
        } else {
            SPSR &= ~_BV(SPI2X);
        }
    }

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
    void enable() const {
        DDRB |= (1 << 2);  // SS   is output
        DDRB |= (1 << 3);  // MOSI is output
        DDRB &= ~(1 << 4); // MISO is input
        DDRB |= (1 << 5);  // SCK  is output
        SPCR = _BV(SPE) | _BV(MSTR); // enable SPI, in master mode
        setClockPrescaler(SPIPrescaler::_2);
    }

    /**
     * Disables SPI mode, releasing pins 10-13 to be used as general purpose I/O.
     */
    void disable() const {
        SPCR &= ~_BV(SPE);
    }

    /**
     * Sends the given byte on MOSI, while reading a byte on MISO and returning that.
     * Blocks until the transfer is complete.
     */
    uint8_t transceive(uint8_t out) const {
        SPDR = out;
        while (!(SPSR & _BV(SPIF))) ;
        return SPDR;
    }

    /**
     * Sends the given byte on MOSI, ignoring anything on MISO. Blocks until the transfer
     * is complete.
     */
    void send(uint8_t out) const {
        SPDR = out;
        while (!(SPSR & _BV(SPIF))) ;
    }
};



#endif /* SPI_HPP_ */
