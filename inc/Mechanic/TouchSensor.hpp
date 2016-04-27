#ifndef MECHANIC_TOUCHSENSOR_HPP_
#define MECHANIC_TOUCHSENSOR_HPP_

#include "AtomicScope.hpp"
#include <Time/Units.hpp>

namespace Mechanic {

namespace Impl {

using namespace Time;

template <typename pin_t>
class TouchSensor {
    pin_t *pin;
public:
    explicit constexpr TouchSensor(pin_t &p): pin(&p) {
        pin->configureAsOutput();
        pin->setLow();
    }

    uint8_t measure() {
        pin->configureAsOutput();
        pin->setLow();
        delay(10_us);
        AtomicScope _;
        pin->configureAsInputWithPullup();

        volatile uint8_t cycles = 17;
        if (pin->isHigh()) cycles = 0;
        else if (pin->isHigh()) cycles = 1;
        else if (pin->isHigh()) cycles = 2;
        else if (pin->isHigh()) cycles = 3;
        else if (pin->isHigh()) cycles = 4;
        else if (pin->isHigh()) cycles = 5;
        else if (pin->isHigh()) cycles = 6;
        else if (pin->isHigh()) cycles = 7;
        else if (pin->isHigh()) cycles = 8;
        else if (pin->isHigh()) cycles = 9;
        else if (pin->isHigh()) cycles = 10;
        else if (pin->isHigh()) cycles = 11;
        else if (pin->isHigh()) cycles = 12;
        else if (pin->isHigh()) cycles = 13;
        else if (pin->isHigh()) cycles = 14;
        else if (pin->isHigh()) cycles = 15;
        else if (pin->isHigh()) cycles = 16;

        pin->configureAsOutput();
        pin->setLow();

        return cycles;
    }
};

} // namespace Impl

template <typename pin_t>
constexpr Impl::TouchSensor<pin_t> TouchSensor(pin_t &pin) {
    return Impl::TouchSensor<pin_t>(pin);
}

} // namespace Mechanic

#endif /* MECHANIC_TOUCHSENSOR_HPP_ */
