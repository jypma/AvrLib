#ifndef PASSIVE_CAPACITORRC_HPP_
#define PASSIVE_CAPACITORRC_HPP_

#include <HAL/Atmel/InterruptHandlers.hpp>
#include <Time/RealTimer.hpp>
#include <Time/Units.hpp>
#include "Logging.hpp"

namespace Passive {

using namespace Time;

namespace Impl {

using namespace Streams;
using namespace HAL::Atmel;
using namespace InterruptHandlers;

enum class CapacitorRCState { IDLE, CHARGING, MEASURING, DISCHARGING };

/**
 * Measures the value of a capacitor connected to a pin and ground, while in parallel with a
 * (typically big) resistor R:
 *
 *
 * [ pin_t ]---+----[ C ]----+----[  GND
 *             |             |
 *             \----[ R ]----/
 *
 * The class will charge the capacitor by pulling the pin high, and then measure the discharge
 * time by switching the pin to a high-impedance input mode.
 */
template <typename rt_t, typename pin_t, uint64_t R,
   typename rise_time = decltype(500_us),
   typename timeout_time = decltype(1_s)
   >
class CapacitorRC {
    static constexpr uint16_t MEASUREMENTS = 128;

    typedef Logging::Log<Loggers::Passive> log;
    typedef CapacitorRC<rt_t, pin_t, R, rise_time, timeout_time> This;
    rt_t* const rt;
    pin_t* const pin;

    CapacitorRCState state = CapacitorRCState::IDLE;
    uint32_t startTime = 0;
    uint32_t time = 0;
    Deadline<rt_t, rise_time> chargeDone = { *rt };
    Deadline<rt_t, timeout_time> timeout = { *rt };
    uint16_t measurements = 0;

    void charge() {
        log::debug(F("charge"));
        state = CapacitorRCState::CHARGING;
        pin->interruptOff();
        pin->configureAsOutputHigh();
        chargeDone.schedule();
        timeout.cancel();
    }

    void idle() {
        log::debug(F("idle"));
        state = CapacitorRCState::IDLE;
        pin->interruptOff();
        pin->configureAsOutputLow();
        chargeDone.cancel();
        timeout.cancel();
        if (measurements > 0) {
            charge();
        } else {
            time /= MEASUREMENTS;
        }
    }

    void doMeasure() {
        log::debug(F("measure"));
        state = CapacitorRCState::MEASURING;
        startTime = rt->counts();
        pin->configureAsInputWithoutPullup();
        chargeDone.cancel();
        if (pin->isHigh()) {
            pin->interruptOnChange();
            timeout.schedule();
        } else {
            time = 0;
            measurements = 0;
            idle();
        }
    }

    void discharge() {
        log::debug(F("discharge"));
        state = CapacitorRCState::DISCHARGING;
        pin->configureAsOutputLow();
        pin->interruptOff();
        chargeDone.schedule();
        timeout.cancel();
    }

    void onPinChange() {
        if (state == CapacitorRCState::MEASURING && measurements > 0) {
            time += (rt->counts() - startTime);
            measurements--;
            discharge();
        } else {
            time = 0;
            measurements = 0;
            idle();
        }
    }
public:
    typedef On<This, typename pin_t::INT, &This::onPinChange> Handlers;

    CapacitorRC(rt_t &_rt, pin_t &_pin): rt(&_rt), pin(&_pin) {
        idle();
    }

    bool isMeasuring() {
        AtomicScope _;
        if (chargeDone.isNow()) {
            if (state == CapacitorRCState::CHARGING) {
                doMeasure();
            } else if (state == CapacitorRCState::DISCHARGING) {
                log::debug(F("after discharge: "), pin->isHigh() ? '1' : '0');
                idle();
            }
        } else if (timeout.isNow()) {
            log::debug(F("timed out"));
            time = rt->counts() - startTime;
            measurements = 0;
            idle();
        }
        return (state != CapacitorRCState::IDLE || measurements > 0);
    }

    bool isIdle() {
        return !isMeasuring();
    }

    CapacitorRCState getState() const {
    	return state;
    }

    const decltype(chargeDone) getChargeDone() const {
    	return chargeDone;
    }

    const decltype(timeout) getTimeout() const {
    	return timeout;
    }

    void measure() {
        AtomicScope _;
        if (isIdle()) {
            time = 0;
            measurements = MEASUREMENTS;
            charge();
        }
    }

    uint32_t getTime() {
        return time;
    }
};

} // namespace Impl

template <
    uint64_t R,
    typename rise_time = decltype(500_us),
    typename timeout_time = decltype(1_s),
    typename rt_t, typename pin_t>
Impl::CapacitorRC<rt_t, pin_t, R, rise_time, timeout_time> CapacitorRC(rt_t &rt, pin_t &pin) {
    return Impl::CapacitorRC<rt_t, pin_t, R, rise_time, timeout_time>(rt, pin);
}

} // namespace Passive



#endif /* PASSIVE_CAPACITORRC_HPP_ */
