/*
 * ADC.hpp
 *
 *  Created on: May 6, 2015
 *      Author: jan
 */

#ifndef ADC_HPP_
#define ADC_HPP_

#include <avr/io.h>
#include <HAL/Atmel/InterruptHandlers.hpp>

namespace HAL {
namespace Atmel {

using namespace InterruptHandlers;

enum class ADReference: uint8_t {
    /** AREF, Internal V ref turned off */
    AREF,
    /** AVCC with external capacitor at AREF pin */
    AVCC,
    /** Internal 1.1V Voltage Reference with external capacitor at AREF pin */
    BANDGAP
};

class ADConverter {
    volatile bool newValue = false;

    void onComplete() {
        newValue = true;
    }

public:
    typedef On<ADConverter, Int_ADC_, &ADConverter::onComplete> Handlers;

    void enable();

    /**
     * Initializes the A/D converter with the reference set to AVCC.
     */
    ADConverter() {
#if (F_CPU == 16000000)
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz sample rate @ 16MHz
#endif
        ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
        ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
        enable();
    }

    void setReference(ADReference ref) {
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

    template <typename pin_t>
    void start(pin_t pin) {
        start<pin_t>();
    }

    template <typename pin_t>
    void start() {
        ADMUX = (ADMUX & ~15) | pin_t::info_t::adc_mux;
        ADCSRA |= (1 << ADSC);  // Start A2D Conversion
    }

    uint16_t awaitValue();

    uint16_t getValue();
};

} // namespace Atmel
} // namespace HAL


#endif /* ADC_HPP_ */
