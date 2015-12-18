#ifndef MECHANIC_BUTTON_HPP_
#define MECHANIC_BUTTON_HPP_

#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"

namespace Mechanic {

using namespace Time;

enum class ButtonEvent: uint8_t {
    IDLE, PRESSED, RELEASED
};

template <typename pin_t, typename rt_t, typename debounce_time = decltype(8_ms)>
class Button {
    pin_t * const pin;
    rt_t * const rt;
    Deadline<rt_t,debounce_time> stopDebouncing;
    bool prevState;
    bool debouncing = false;
public:
    Button(pin_t &_pin, rt_t &_rt): pin(&_pin), rt(&_rt), stopDebouncing(deadline(_rt, debounce_time::instance)) {
        pin->configureAsInputWithPullup();
        prevState = pin->isHigh();
    }

    ButtonEvent nextEvent() {
        if (stopDebouncing.isNow()) {
            debouncing = false;
        }
        if (debouncing) {
            return ButtonEvent::IDLE;
        }
        const bool state = pin->isHigh();
        if (state == prevState) {
            return ButtonEvent::IDLE;
        } else {
            prevState = state;
            debouncing = true;
            stopDebouncing.reset();
            return state ? ButtonEvent::PRESSED : ButtonEvent::RELEASED;
        }
    }
};

}



#endif /* MECHANIC_BUTTON_HPP_ */
