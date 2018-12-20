/*
 * ADC.hpp
 *
 *  Created on: May 6, 2015
 *      Author: jan
 */

#ifndef ADC_HPP_
#define ADC_HPP_

#include <HAL/Atmel/Registers.hpp>
#include <HAL/Atmel/InterruptHandlers.hpp>
#include "AtomicScope.hpp"
#include <stdint.h>

namespace HAL {
namespace Atmel {

using namespace InterruptHandlers;
using namespace Registers;

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

    template <typename pin_t>
    void selectPin() {
      ADMUX.apply(pin_t::info_t::adc_mux);
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
        selectPin<pin_t>();
        ADCSRA |= ADSC;  // Start A2D Conversion
    }

    template <typename pin_t>
    void measureContinuously(pin_t pin) {
        measureContinuously<pin_t>();
    }

    template <typename pin_t>
    void measureContinuously() {
        selectPin<pin_t>();
        ADCSRA |= ADATE; //enabble auto trigger
        ADCSRA |= ADSC;  // Start A2D Conversion
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
    	// Set ADC prescaler to 128: 125KHz sample rate @ 16MHz
    	ADPS2.set();
    	ADPS1.set();
    	ADPS0.set();
        //ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
#endif
        ADLAR.clear();
    }

    uint16_t awaitValue(){
        while (ADSC.isSet()) ;
        return getValue();
    }

    uint16_t getValue() {
        AtomicScope _;
        return ADC.get();
    }
};

template <>
class ADConverter<uint8_t>: public Impl::BaseADC {
public:
    using BaseADC::Handlers;

    ADConverter() {
#if (F_CPU == 16000000)
    	// Set ADC prescaler to 32: 500KHz ADC rate -> 40kHz sampling rate
    	ADPS2.set();
    	ADPS1.clear();
    	ADPS0.set();
        //ADCSRA |= (1 << ADPS2) | (1 << ADPS0);
#endif

    	ADLAR.set();    // Using only 8 bits in ADCH register
        //ADMUX |= (1 << ADLAR);
    }

    uint8_t awaitValue() {
        while (ADSC.isSet()) ;
        return getValue();
    }

    uint8_t getValue() {
    	return ADCH.get();
    }
};

} // namespace Atmel
} // namespace HAL


#endif /* ADC_HPP_ */
