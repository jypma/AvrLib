#include "ADConverter.hpp"
#include "AtomicScope.hpp"

InterruptChain _adc_onConversionComplete;

ISR(ADC_vect)
{
    _adc_onConversionComplete.invoke();
}

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
