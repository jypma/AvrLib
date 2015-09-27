#include "HAL/Atmel/ADConverter.hpp"
#include "AtomicScope.hpp"

using namespace HAL::Atmel;

void ADConverter::enable() {
    ADCSRA |= (1 << ADEN);  // Enable ADC
}

uint16_t ADConverter::awaitValue() {
    while (ADCSRA & (1 << ADSC)) ;
    return getValue();
}

uint16_t ADConverter::getValue() {
    AtomicScope _;
    return ADC;
}
