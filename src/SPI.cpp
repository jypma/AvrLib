#include "SPI.hpp"

void SPIMaster::setClockPrescaler(SPIPrescaler p) const {
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

void SPIMaster::enable() const {
    DDRB |= (1 << 2);  // SS   is output
    DDRB |= (1 << 3);  // MOSI is output
    DDRB &= ~(1 << 4); // MISO is input
    DDRB |= (1 << 5);  // SCK  is output
    SPCR = _BV(SPE) | _BV(MSTR); // enable SPI, in master mode
    setClockPrescaler(SPIPrescaler::_2);
}

uint8_t SPIMaster::transceive(uint8_t out) const {
    SPDR = out;
    while (!(SPSR & _BV(SPIF))) ;
    return SPDR;
}

void SPIMaster::send(uint8_t out) const {
    SPDR = out;
    while (!(SPSR & _BV(SPIF))) ;
}
