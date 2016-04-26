#ifndef PASSIVE_SUPPLYVOLTAGE_HPP_
#define PASSIVE_SUPPLYVOLTAGE_HPP_

#include "EEPROM.hpp"
#include "HAL/Atmel/ADConverter.hpp"

namespace Passive {

using namespace HAL::Atmel;

namespace Impl {

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
template <typename adc_t, typename pin_t, uint16_t R1, uint16_t R2, uint16_t EEPROM::*bandgapVoltage>
class SupplyVoltage {
    adc_t *adc;
public:
    SupplyVoltage(adc_t &_adc): adc(&_adc) {}

    /**
     * Performs an A/D conversion and returns the measured supply voltage in mV, calculated
     * using the configured R1 and R2 resistors.
     */
    uint16_t get() {
        adc->setReference(ADReference::BANDGAP);
        adc->template start<pin_t>();
        uint16_t result = adc->awaitValue();
        //return result;
        return uint32_t(read(bandgapVoltage)) * result / 1024 * (R1 + R2) / R2;
    }
};

}

template <uint16_t R1, uint16_t R2, uint16_t EEPROM::*bandgapVoltage, typename adc_t, typename pin_t>
Impl::SupplyVoltage<adc_t, pin_t, R1, R2, bandgapVoltage> SupplyVoltage(adc_t &adc, pin_t pin) {
    return Impl::SupplyVoltage<adc_t, pin_t, R1, R2, bandgapVoltage>(adc);
}

}



#endif /* PASSIVE_SUPPLYVOLTAGE_HPP_ */
