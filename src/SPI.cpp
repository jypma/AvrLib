#include "HAL/Atmel/SPI.hpp"

using namespace HAL::Atmel;

void SPIMaster::setClockPrescaler(SPIPrescaler p) const {
    uint8_t bits = static_cast<uint8_t>(p);
    SPR0.apply(bits & 1);
    SPR1.apply(bits & 2);
    SPI2X.apply(bits & 4);
}

void SPIMaster::enable() const {
	DDB2.set();   // SS   is output
	DDB3.set();   // MOSI is output
	DDB4.clear(); // MISO is input
	DDB5.set();   // SCK  is output;
        SPIE.clear(); // no interrupts
	MSTR.set();   // in master mode
	SPE.set();    // enable SPI
    setClockPrescaler(SPIPrescaler::_2);
}

uint8_t SPIMaster::transceive(uint8_t out) const {
    SPDR.val() = out;
    while (SPIF.isCleared()) ;
    return SPDR.val();
}

void SPIMaster::send(uint8_t out) const {
    SPDR.val() = out;
    while (SPIF.isCleared()) ;
}
        
