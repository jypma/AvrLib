/*
 * SupplyVoltage.cpp
 *
 *  Created on: Nov 19, 2014
 *      Author: jan
 */

#include "SupplyVoltage.hpp"
/*
SupplyVoltage::SupplyVoltage(uint8_t const _pin, uint16_t const _r1, uint16_t const _r2, uint16_t const _vcc_bandgap):
  pin(_pin), r1(_r1), r2(_r2), vcc_bandgap(_vcc_bandgap) {

}

uint16_t SupplyVoltage::get() {
    ADMUX = (1<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (pin - A0);
    uint32_t adc;
    for (int i = 0; i < 3; i++) {
        ADCSRA |= _BV( ADSC );
        // Wait for it to complete
        while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
        adc = ADC;
    }
    // Scale the value
    uint32_t voltage = adc * vcc_bandgap * (r1 + r2) / r2 / 1023; // in 0.001 volt
#ifdef DEBUG
    Serial.print("VCC:");
    Serial.println(voltage);
#endif
    return voltage;

}
*/
