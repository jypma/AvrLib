#include "HAL/Atmel/ADConverter.hpp"
#include "AtomicScope.hpp"

using namespace HAL::Atmel;

Impl::BaseADC::BaseADC() {
	REFS0.set();   // Set ADC reference to AVCC
	ADIE.set();    // Enable ADC Interrupt
    enable();
}

void Impl::BaseADC::enable() {
	ADEN.set(); // Enable ADC
}

void Impl::BaseADC::setReference(ADReference ref) {
    switch (ref) {
    case ADReference::AREF:
    	REFS0.clear();
    	REFS1.clear();
        //ADMUX &= ~(_BV(REFS0) | _BV(REFS1));
        break;
    case ADReference::AVCC:
    	REFS0.set();
    	REFS1.clear();
        //ADMUX = (ADMUX | _BV(REFS0)) & ~_BV(REFS1);
        break;
    case ADReference::BANDGAP:
    	REFS0.set();
    	REFS1.set();
        //ADMUX |= _BV(REFS0) | _BV(REFS1);
        break;
    }
}


