#ifndef MECHANIC_BUTTON_HPP_
#define MECHANIC_BUTTON_HPP_

#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "AtomicScope.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"

namespace Mechanic {

using namespace Time;
using namespace HAL::Atmel;
using namespace InterruptHandlers;

/**
 * Indicates events that can happen to the button.
 */
enum class ButtonEvent: uint8_t {
    /** The button is currently being held down (and was so on the last call to nextEvent()) */
    DOWN,
    /** The button is currently up (and was so on the last call to nextEvent()) */
    UP,
    /** The button was pressed since the last call to nextEvent() */
    PRESSED,
    /** The button was released since the last call to nextEvent() */
    RELEASED
};


namespace Impl {

/**
 * Reads a button including debouncing, and configuring to wake the MCU from sleep.
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
    }

    bool getState() {
        AtomicScope _;
        bool state = pin->isHigh();
        if (gotInterrupt && state == prevState) {
        	state = !prevState;
        }
		gotInterrupt = false;
		return state;
    }
public:
    typedef On<This, typename pin_t::INT, &This::onInterrupt> Handlers;

    Button(pin_t &_pin, rt_t &_rt): pin(&_pin), rt(&_rt), stopDebouncing(deadline(_rt, debounce_time::instance)) {
        pin->configureAsInputWithPullup();
        pin->interruptOnChange();
        prevState = pin->isHigh();
    }

    /** Returns a ButtonEvent indicating any event that happened since this method was last called. */
    ButtonEvent nextEvent() {
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
            stopDebouncing.schedule();
            return state ? ButtonEvent::RELEASED : ButtonEvent::PRESSED;
        }
    }

    /** Returns whether the button is up (not being pressed). */
    bool isUp() {
        return prevState;
    }

    /** Returns whether the button is currently being held down. */
    bool isDown() {
        return !isUp();
    }
};

}

template <typename pin_t, typename rt_t>
Impl::Button<pin_t, rt_t, decltype(8_ms)> Button(rt_t &rt, pin_t &pin) {
    return Impl::Button<pin_t, rt_t, decltype(8_ms)>(pin, rt);
}

template <typename pin_t, typename rt_t, typename debounce_time>
Impl::Button<pin_t, rt_t, debounce_time> Button(rt_t &rt, pin_t &pin, debounce_time d) {
    return Impl::Button<pin_t, rt_t, debounce_time>(pin, rt);
}

}



#endif /* MECHANIC_BUTTON_HPP_ */
