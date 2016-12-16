#ifndef PASSIVE_CAPACITORRC_HPP_
#define PASSIVE_CAPACITORRC_HPP_

#include <HAL/Atmel/InterruptHandlers.hpp>
#include <Time/RealTimer.hpp>
#include <Time/Units.hpp>
#include "Logging.hpp"
#include "Option.hpp"
#include <Tasks/TaskState.hpp>

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
    Option<uint32_t> time;
    VariableDeadline<rt_t> timeout = { *rt };
    uint16_t measurements = 0;

    void charge() {
        log::debug(F("charge"));
        state = CapacitorRCState::CHARGING;
        pin->interruptOff();
        pin->configureAsOutputHigh();
        timeout.schedule(rise_time::instance);
    }

    void idle() {
        log::debug(F("idle"));
        state = CapacitorRCState::IDLE;
        pin->interruptOff();
        pin->configureAsOutputLow();
        timeout.cancel();
        if (measurements > 0) {
            charge();
        } else {
            time /= MEASUREMENTS;
            if (time < (MEASUREMENTS * uint32_t(4))) { // charged on average within 4 cycles, that's too fast to be useful.
            	time = none();
            }
        }
    }

    void doMeasure() {
        log::debug(F("measure"));
        state = CapacitorRCState::MEASURING;
        startTime = rt->counts();
        pin->configureAsInputWithoutPullup();
        if (pin->isHigh()) {
            pin->interruptOnChange();
            timeout.schedule(timeout_time::instance);
        } else {
            time = none();
            measurements = 0;
            idle();
        }
    }

    void discharge() {
        log::debug(F("discharge"));
        state = CapacitorRCState::DISCHARGING;
        pin->configureAsOutputLow();
        pin->interruptOff();
        timeout.schedule(rise_time::instance);
    }

    void onPinChange() {
        if (state == CapacitorRCState::MEASURING && measurements > 0) {
            time += (rt->counts() - startTime);
            measurements--;
            discharge();
        } else {
            time = none();
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
        if (timeout.isNow()) {
        	switch(state) {
        	case CapacitorRCState::CHARGING:
                doMeasure();
        		break;
        	case CapacitorRCState::DISCHARGING:
                log::debug(F("after discharge: "), pin->isHigh() ? '1' : '0');
                idle();
                break;
        	default:
                log::debug(F("timed out"));
                time = none();
                measurements = 0;
                idle();
        	}
        }
        return (state != CapacitorRCState::IDLE || measurements > 0);
    }

    bool isIdle() {
        return !isMeasuring();
    }

    CapacitorRCState getState() const {
    	return state;
    }

    const auto timeLeft() const {
    	return timeout.timeLeft();
    }

    void measure() {
        AtomicScope _;
        if (isIdle()) {
            time = 0;
            measurements = MEASUREMENTS;
            charge();
        }
    }

    Option<uint32_t> getValue() const {
        return time;
    }

    ::Impl::TaskState<Counts> getTaskState() {
    	AtomicScope _;
    	isMeasuring();
    	switch (state) {
    	case CapacitorRCState::CHARGING:
    	case CapacitorRCState::DISCHARGING:
    		return TaskState(timeout, HAL::Atmel::SleepMode::POWER_DOWN);
    	case CapacitorRCState::MEASURING:
    		return TaskState(timeout, HAL::Atmel::SleepMode::IDLE); // we need the timers to count
    	default:
    		return TaskStateIdle<Counts>();
    	}
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
