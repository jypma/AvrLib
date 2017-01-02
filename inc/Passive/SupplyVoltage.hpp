#ifndef PASSIVE_SUPPLYVOLTAGE_HPP_
#define PASSIVE_SUPPLYVOLTAGE_HPP_

#include "EEPROM.hpp"
#include "HAL/Atmel/ADConverter.hpp"
#include "Logging.hpp"
#include <avr/sleep.h>
#include <util/atomic.h>

namespace Passive {

using namespace HAL::Atmel;

namespace Impl {

using namespace Streams;

/**
 * Calculates the supply / battery voltage, assuming it's connected to one of the analog
 * inputs using the following voltage divider:
 *
 *    VBAT ---[ R1 ]---+---[ R2 ]--- GND
 *                     |
 *               analog input
 *
 * It measures this against the internal bandgap voltage of about 1V. This means that the analog input
 * must not exceed 1V. Pick your R1 and R2 accordingly.
 */
template <uint16_t R1, uint16_t R2, uint16_t EEPROM::*bandgapVoltage, typename adc_t, typename pin_t>
class SupplyVoltage {
    typedef Logging::Log<Loggers::Passive> log;
    adc_t *adc;
    static constexpr uint8_t lowReadingsNeeded = 100; // number of low readings before stop
    uint8_t lowReadingsUntilStop = lowReadingsNeeded;
public:
    SupplyVoltage(adc_t &_adc): adc(&_adc) {}

    /**
     * Performs an A/D conversion and returns the measured supply voltage in mV, calculated
     * using the configured R1 and R2 resistors.
     */
    uint16_t get() {
        adc->setReference(ADReference::BANDGAP);
        adc->template measure<pin_t>();
        uint16_t result = adc->awaitValue();
        //return result;
        auto bg = read(bandgapVoltage);
        log::debug(F("bg="), dec(bg), F(" result="), dec(result), F("R1="), dec(R1), F("R2="), dec(R2));
        return uint32_t(bg) * result / 1024 * (R1 + R2) / R2;
    }

    /**
     * Executes the given lambda, and then turns off the microcontroller, if the battery voltage
     * is less than the given threshold in mV.
     */
    template <typename lambda_t>
    void stopOnLowBattery(uint16_t threshold, lambda_t onShutdown) {
        const uint8_t oldADCSRA = ADCSRA;
        const uint8_t oldPCICR = PCICR;
        const uint8_t oldEIMSK = EIMSK;

        while (true) {
        	bool low = true;
			for (uint8_t count = 4; count > 0; count--) {
				auto value = get();
				if (value >= threshold || value < 1500) {
					low = false;
					lowReadingsUntilStop = lowReadingsNeeded;
					break;
				}
			}
			if (low) {
				if (lowReadingsUntilStop > 0) {
					lowReadingsUntilStop--;
					return;
				}
        	} else {
        		return;
        	}

            onShutdown();

            ADCSRA &= ~ _BV(ADEN); // disable the ADC
            PCICR &= ~(_BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2)); // disable pin-change interrupts
            EIMSK &= ~(_BV(INT0) | _BV(INT1)); // disable hardware interrupts

            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
            ATOMIC_BLOCK(ATOMIC_FORCEON) {
                sleep_enable();
                sleep_bod_disable();
            }
            sleep_cpu();

            // If by some magic, we manage to wake up, restore and re-measure
            lowReadingsUntilStop = lowReadingsNeeded;
            ADCSRA = oldADCSRA;
            PCICR = oldPCICR;
            EIMSK = oldEIMSK;
        }

    }

    /**
     * Turns off the microcontroller if the battery voltage is less than the given amount, in mV.
     */
    void stopOnLowBattery(uint16_t threshold) {
        stopOnLowBattery(threshold, [] {});
    }

};

}

template <uint16_t R1, uint16_t R2, uint16_t EEPROM::*bandgapVoltage, typename adc_t, typename pin_t>
Impl::SupplyVoltage<R1, R2, bandgapVoltage, adc_t, pin_t> SupplyVoltage(adc_t &adc, pin_t pin) {
    return Impl::SupplyVoltage<R1, R2, bandgapVoltage, adc_t, pin_t>(adc);
}

}



#endif /* PASSIVE_SUPPLYVOLTAGE_HPP_ */
