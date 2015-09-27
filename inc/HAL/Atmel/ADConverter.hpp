/*
 * ADC.hpp
 *
 *  Created on: May 6, 2015
 *      Author: jan
 */

#ifndef ADC_HPP_
#define ADC_HPP_

#include <avr/io.h>

namespace HAL {
namespace Atmel {

class ADConverter {
public:
    void enable();

    ADConverter() {
#if (F_CPU == 16000000)
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz sample rate @ 16MHz
#endif
        ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
        ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
        enable();
    }

    template <typename pin_t>
    void start(pin_t pin) {
        ADMUX = (ADMUX & ~15) | pin_t::info_t::adc_mux;
        ADCSRA |= (1 << ADSC);  // Start A2D Conversion
    }

    uint16_t awaitValue();

    uint16_t getValue();
};

} // namespace Atmel
} // namespace HAL


#endif /* ADC_HPP_ */
