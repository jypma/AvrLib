/*
 * ADC.hpp
 *
 *  Created on: May 6, 2015
 *      Author: jan
 */

#ifndef ADC_HPP_
#define ADC_HPP_

#include <avr/io.h>
#include "AtomicScope.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"

namespace HAL {
namespace Atmel {

enum class ADReference: uint8_t {
    /** AREF, Internal V ref turned off */
    AREF,
    /** AVCC with external capacitor at AREF pin */
    AVCC,
    /** Internal 1.1V Voltage Reference with external capacitor at AREF pin */
    BANDGAP
};

namespace Impl {

using namespace InterruptHandlers;

class BaseADC {
    volatile bool newValue = false;

    void onComplete() {
        newValue = true;
    }

public:
    typedef On<BaseADC, Int_ADC_, &BaseADC::onComplete> Handlers;
    /**
     * Initializes the A/D converter with the reference set to AVCC.
     */
    BaseADC();

    bool hasNewValue() {
    	bool n = newValue;
    	newValue = false;
    	return n;
    }

    /**
     * Enabled the A/D converter (enabled by default in constructor)
     */
    void enable();

    void setReference(ADReference ref);

    template <typename pin_t>
    void measure(pin_t pin) {
    	measure<pin_t>();
    }

    template <typename pin_t>
    void measure() {
        ADMUX = (ADMUX & ~15) | pin_t::info_t::adc_mux;
        ADCSRA |= (1 << ADSC);  // Start A2D Conversion
    }

    template <typename pin_t>
    void measureContinuously(pin_t pin) {
        measureContinuously<pin_t>();
    }

    template <typename pin_t>
    void measureContinuously() {
        ADMUX = (ADMUX & ~15) | pin_t::info_t::adc_mux;
        ADCSRA |= (1 << ADATE); //enabble auto trigger
        ADCSRA |= (1 << ADSC);  // Start A2D Conversion
    }
};

}

template <typename size_t>
class ADConverter;

template <>
class ADConverter<uint16_t>: public Impl::BaseADC {
public:
    using BaseADC::Handlers;

    ADConverter() {
#if (F_CPU == 16000000)
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz ADC rate -> 10kHz sampling rate
#endif
    }

    uint16_t awaitValue(){
        while (ADCSRA & (1 << ADSC)) ;
        return getValue();
    }

    uint16_t getValue() {
        AtomicScope _;
        return ADC;
    }
};

template <>
class ADConverter<uint8_t>: public Impl::BaseADC {
public:
    using BaseADC::Handlers;

    ADConverter() {
#if (F_CPU == 16000000)
        ADCSRA |= (1 << ADPS2) | (1 << ADPS0);// Set ADC prescaler to 32 - 500KHz ADC rate -> 40kHz sampling rate
#endif
        ADMUX |= (1 << ADLAR); // Using only 8 bits in ADCH register
    }

    uint8_t awaitValue() {
        while (ADCSRA & (1 << ADSC)) ;
        return getValue();
    }

    uint8_t getValue() {
    	return ADCH;
    }
};

} // namespace Atmel
} // namespace HAL


#endif /* ADC_HPP_ */
