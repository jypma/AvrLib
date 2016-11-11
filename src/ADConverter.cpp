#include "HAL/Atmel/ADConverter.hpp"
#include "AtomicScope.hpp"

using namespace HAL::Atmel;

Impl::BaseADC::BaseADC() {
    ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
    ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
    enable();
}

void Impl::BaseADC::enable() {
    ADCSRA |= (1 << ADEN);  // Enable ADC
}

void Impl::BaseADC::setReference(ADReference ref) {
    switch (ref) {
    case ADReference::AREF:
        ADMUX &= ~(_BV(REFS0) | _BV(REFS1));
        break;
    case ADReference::AVCC:
        ADMUX = (ADMUX | _BV(REFS0)) & ~_BV(REFS1);
        break;
    case ADReference::BANDGAP:
        ADMUX |= _BV(REFS0) | _BV(REFS1);
        break;
    }
}


