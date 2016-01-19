#ifndef MECHANIC_BUTTON_HPP_
#define MECHANIC_BUTTON_HPP_

#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "AtomicScope.hpp"
#include "HAL/Atmel/InterruptVectors.hpp"

namespace Mechanic {

using namespace Time;

enum class ButtonEvent: uint8_t {
    DOWN, UP, PRESSED, RELEASED
};

/**
 * Reads a button including debouncing, and configuring to wake the MCU from sleep. If you put a button
 * on INT0 or INT1, don't put the MCU into POWER_DOWN until nextEvent() has informed you that the
 * button was released.
 */
template <typename pin_t, typename rt_t, typename debounce_time = decltype(8_ms)>
class Button {
    typedef Button<pin_t, rt_t, debounce_time> This;

    pin_t * const pin;
    rt_t * const rt;
    Deadline<rt_t,debounce_time> stopDebouncing;
    bool prevState;
    bool debouncing = false;
    volatile bool gotInterrupt = false;

    void onInterrupt() {
        gotInterrupt = true;
        pin->interruptOff(); // disable interrupt handler so we don't hang around in here while the user holds down the button
    }

    bool getState() {
        AtomicScope _;
        if (gotInterrupt) {
            gotInterrupt = false;
            return false;
        } else {
            return pin->isHigh();
        }
    }
public:
    Button(pin_t &_pin, rt_t &_rt): pin(&_pin), rt(&_rt), stopDebouncing(deadline(_rt, debounce_time::instance)) {
        pin->configureAsInputWithPullup();
        pin->interruptOnLow();
        prevState = pin->isHigh();
    }

    ButtonEvent nextEvent() {
        if (pin->isHigh()) {
            // button was released, re-enable interrupt
            pin->interruptOnLow();
        }

        if (stopDebouncing.isNow()) {
            debouncing = false;
        }
        if (debouncing) {
            return prevState ? ButtonEvent::UP : ButtonEvent::DOWN;
        }
        const bool state = getState();
        if (state == prevState) {
            return prevState ? ButtonEvent::UP : ButtonEvent::DOWN;
        } else {
            prevState = state;
            debouncing = true;
            stopDebouncing.reset();
            return state ? ButtonEvent::RELEASED : ButtonEvent::PRESSED;
        }
    }

    INTERRUPT_HANDLER1(typename pin_t::INT, onInterrupt);
};

template <typename pin_t, typename rt_t>
Button<pin_t, rt_t, decltype(8_ms)> button(rt_t &rt, pin_t &pin) {
    return Button<pin_t, rt_t, decltype(8_ms)>(pin, rt);
}

template <typename pin_t, typename rt_t, typename debounce_time>
Button<pin_t, rt_t, debounce_time> button(rt_t &rt, pin_t &pin, debounce_time d) {
    return Button<pin_t, rt_t, debounce_time>(pin, rt);
}

}



#endif /* MECHANIC_BUTTON_HPP_ */
