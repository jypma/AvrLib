#ifndef PIR_HCSR501_HPP_
#define PIR_HCSR501_HPP_

#include "AtomicScope.hpp"
#include "Time/RealTimer.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"

namespace PIR {

using namespace Time;

enum class HCSR501State { OFF, INITIALIZING, READY, SLEEPING };

namespace Impl {

using namespace HAL::Atmel::InterruptHandlers;

/**
 * Provides a nice interface to the HC-SR501 Passive Infrared (PIR) sensor module,
 * which can be readily found on the customary market places.
 *
 *      /-------------------------------------------------\
 *      |                                                 |
 *      |                     HC-SR501                    |
 *      |                  (component side)               |
 *      |                                                 |
 *      |                                                 |
 *      |                /--------------------\           |
 *      |  C1            | VCC  | DATA | GND  |      C2   |
 *      \----------------+------+------+------+-----------/
 *
 * Instead of relying on its built-in delay mechanism, we power the module through a microcontroller pin,
 * in order to save power during the sleep time after having detected movement.
 *
 * By default, the module is somewhat power-inefficient. It expects 5V on VCC and drops this through a diode and
 * a linear regulator to 3.3V. In order to power it from 3.3V directly, just connect the positive leads of
 * C1 and C2, as indicated in the drawing above.
 *
 * DATA is expected to turn high for about 1 second to indicate an event.
 *
 * @param datapin_t  Pin that DATA is connected to (must have interrupt or pin change interrupt capability)
 * @param powerpin_t Pin that VCC is connected to
 * @param delay      Time to turn off after having detected motion (to conserve battery)
 */
template <typename datapin_t, typename powerpin_t, typename rt_t, typename delay>
class HCSR501 {
    typedef HCSR501<datapin_t, powerpin_t, rt_t, delay> This;
    typedef HCSR501State State;

    datapin_t *pin;
    powerpin_t *power;
    VariableDeadline<rt_t> timeout;
    volatile State state = State::OFF;
    volatile bool motion = false;

    void initializing() {
        if (timeout.isNow()) {
            state = State::READY;
            pin->interruptOnRising();
        }
    }

    void onPinRising() {
        if (state == State::READY) {
            state = State::SLEEPING;
            motion = true;
            pin->interruptOff();
            power->setLow();
            timeout.template schedule<delay>();
        }
    }

    void turnPowerOn() {
        motion = false;
        power->setHigh();
        state = State::INITIALIZING;
        timeout.schedule(5_s);
    }

    void sleeping() {
        if (timeout.isNow()) {
            turnPowerOn();
        }
    }

public:
    typedef On<This, typename datapin_t::INT, &This::onPinRising> Handlers;

    HCSR501(datapin_t &_pin, powerpin_t &_power, rt_t &_rt):
        pin(&_pin),
        power(&_power),
        timeout(deadline(_rt)) {
        power->configureAsOutput();
        pin->configureAsInputWithPullup();
        pin->interruptOff();
        turnPowerOn();
    }

    HCSR501State getState() const {
        return state;
    }

    void powerOn() {
        if (state == State::OFF) {
            turnPowerOn();
        }
    }

    void loop() {
        switch (state) {
        case State::OFF: break;
        case State::INITIALIZING: initializing(); break;
        case State::READY: break; // no looping here, we process by interrupt
        case State::SLEEPING: sleeping(); break;
        }
    }

    bool isMotionDetected() {
        AtomicScope _;

        bool result = motion;
        motion = false;
        return result;
    }
};

} // namespace Impl

/**
 * Declares a HCSR501 with the given sleep delay
 */
template <typename datapin_t, typename powerpin_t, typename rt_t, typename delay_t>
Impl::HCSR501<datapin_t, powerpin_t, rt_t, delay_t> HCSR501(datapin_t &data, powerpin_t &power, rt_t &rt, delay_t delay) {
    return { data, power, rt };
}

/**
 * Declares a HCSR501 with a 15-minute sleep delay
 */
template <typename datapin_t, typename powerpin_t, typename rt_t>
Impl::HCSR501<datapin_t, powerpin_t, rt_t, decltype(14_min)> HCSR501(datapin_t &data, powerpin_t &power, rt_t &rt) {
    return { data, power, rt };
}


} // namespace PIR



#endif /* PIR_HCSR501_HPP_ */
