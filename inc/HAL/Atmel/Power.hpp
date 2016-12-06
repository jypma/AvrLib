#ifndef HAL_ATMEL_POWER_HPP_
#define HAL_ATMEL_POWER_HPP_

#include "SleepMode.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "AtomicScope.hpp"
#include "HAL/Atmel/Device.hpp"
#include "Time/Units.hpp"
#include <util/atomic.h>
#include <avr/sleep.h>
#include "Tasks/TaskState.hpp"
#include "Logging.hpp"

namespace HAL {
namespace Atmel {

using namespace Time;
using namespace InterruptHandlers;

/**
 * Returns whether s1 indicates a deeper sleep than s2.
 */
inline bool operator > (SleepMode s1, SleepMode s2) {
	return ((uint8_t)s1) > ((uint8_t)s2);
}

enum class SleepGranularity: uint8_t {
    _16ms = 0,
    _32ms = 1,
    _64ms = 2,
    _125ms = 3,
    _250ms = 4,
    _500ms = 5,
    _1000ms = 6,
    _2000ms = 7,
    _4000ms = 8,
    _8000ms = 9
};

namespace Impl {

template <typename rt_t>
class Power {
    typedef Power<rt_t> This;
    typedef Logging::Log<Loggers::Power> log;

    rt_t *rt;
    volatile uint8_t _watchdogCounter;

    void setWatchdogInterrupts (int8_t mode) {
        // correct for the fact that WDP3 is *not* in bit position 3!
        if (mode & _BV(3))
            mode ^= _BV(3) | _BV(WDP3);
        // pre-calculate the WDTCSR value, can't do it inside the timed sequence
        // we only generate interrupts, no reset
        uint8_t wdtcsr = mode >= 0 ? _BV(WDIE) | mode : 0;
        MCUSR &= ~(1<<WDRF);
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
    #ifndef WDTCSR
    #define WDTCSR WDTCR
    #endif
            WDTCSR |= (1<<WDCE) | (1<<WDE); // timed sequence
            WDTCSR = wdtcsr;
        }
    }

    void onWatchdog() {
        _watchdogCounter++;
    }

    bool doSleepFor(Milliseconds<> ms, SleepMode mode, SleepGranularity maxGranularity) {
    	log::debug(F("Z: "), dec(ms.getValue()), F("ms in "), '0' + uint8_t(mode));
    	log::flush();

        bool interrupted = false;
        uint32_t millisSleep = ms.getValue();
        if (millisSleep <= 16) {
            return false;
        }

        uint32_t msleft = millisSleep;
        // only slow down for periods longer than the watchdog granularity
        while (msleft >= 16) {
            uint8_t wdp = 0; // wdp 0..9 corresponds to roughly 16..8192 ms
            // calc wdp as log2(msleft/16), i.e. loop & inc while next value is ok
            for (uint32_t m = msleft; m >= 32; m >>= 1)
                if (++wdp >= uint8_t(maxGranularity))
                    break;
            _watchdogCounter = 0;
            setWatchdogInterrupts(wdp);
            sleep(mode);
            setWatchdogInterrupts(-1); // off
            // when interrupted, our best guess is that half the time has passed
            uint32_t halfms = 8 << wdp;
            msleft -= halfms;
            if (_watchdogCounter == 0) {
                interrupted = true; // lost some time, but got interrupted
                break;
            }
            msleft -= halfms;
        }

        if (mode != SleepMode::IDLE) {
            // adjust ticks for the delay we've just had. Not for IDLE, since timers keep running
            // there.
            millisSleep -= msleft;
            auto ms = Milliseconds<>(millisSleep);
            rt->haveSlept(ms);
        }

        log::debug(F("W: "), '0' + interrupted);
        return interrupted;
    }

    bool sleepUntilAnyLT(Milliseconds<> timeout, SleepMode mode) {
        return doSleepFor(timeout, mode, SleepGranularity::_8000ms);
    }

    template <typename periodic_t, typename... periodic_ts>
    bool sleepUntilAnyLT(Milliseconds<> timeout, SleepMode mode, const periodic_t &head, const periodic_ts&... tail) {
        auto headTime = toMillisOn<rt_t>(head.timeLeft());
        if (headTime < timeout) {
            return sleepUntilAnyLT(headTime, mode, tail...);
        } else {
            return sleepUntilAnyLT(timeout, mode, tail...);
        }
    }

public:
    typedef On<This, Int_WDT_, &This::onWatchdog> Handlers;

    Power(rt_t &_rt): rt(&_rt) {}

    /**
     * Powers down (sleeps) into the given sleep mode, until a hardware interrupt wakes the microcontroller up again.
     * Which hardware interrupts are available, and how long it takes to wake up, depends on the sleep mode.
     */
    void sleep(SleepMode mode) {
        uint8_t adcsraSave = ADCSRA;
        ADCSRA &= ~ _BV(ADEN); // disable the ADC
        switch(mode) {
        case SleepMode::POWER_DOWN:
            set_sleep_mode(SLEEP_MODE_PWR_DOWN); break;
        case SleepMode::STANDBY:
            set_sleep_mode(SLEEP_MODE_STANDBY); break;
        case SleepMode::IDLE:
            set_sleep_mode(SLEEP_MODE_IDLE); break;
        }

        ATOMIC_BLOCK(ATOMIC_FORCEON) {
            sleep_enable();
            sleep_bod_disable();
        }
        sleep_cpu();
        sleep_disable();
        // re-enable what we disabled
        ADCSRA = adcsraSave;
    }

    /**
     * Attempts to sleep (power down) for at most until the given Deadline or Periodic timer fires.
     * A hardware or pin change interrupt can cause premature wake-up.
     * Sleeping is performed by sleeping multiple consecutive watchdog timer intervals. If an interrupt occurs
     * during a watchdog timer interval, the function guesses that 1/2 of that interval's time has elapsed. This causes
     * real-time keeping to go out of sync by, on average 1/2 of the interval time. Set [maxGranularity] to the
     * highest interval that's acceptable (higher values means less power used).
     *
     * Returns whether any hardware interrupts have interrupted sleep.
     */
    template <typename periodic_t>
    bool sleepUntil(const periodic_t &p, SleepMode mode, SleepGranularity maxGranularity) {
        return sleepFor(p.timeLeft(), mode, maxGranularity);
    }

    template <typename periodic_t>
    bool sleepUntil(const periodic_t &p, SleepMode mode) {
        return sleepFor(p.timeLeft(), mode, SleepGranularity::_8000ms);
    }

    // FIXME assert that the periodics are of the same rt_t
    /**
     * Sleeps until the EARLIEST of the given periodic or deadline instances fires.
     */
    template <typename periodic_t, typename... periodic_ts>
    bool sleepUntilAny(SleepMode mode, const periodic_t &head, const periodic_ts&... tail) {
        return sleepUntilAnyLT(toMillisOn<rt_t>(head.timeLeft()), mode, tail...);
    }

    bool sleepUntilTasks() {
    	// all tasks are idle... programmer error. let's not sleep just to be safe.
    	return false;
    }

    template <typename time_t>
    bool sleepUntilTasks(::Impl::TaskState<time_t> task) {
    	return sleepFor(task.timeLeft(), task.getMaxSleepMode(), SleepGranularity::_8000ms);
    }

    template <typename time1_t, typename time2_t, typename... types>
    bool sleepUntilTasks(::Impl::TaskState<time1_t> task1, ::Impl::TaskState<time2_t> task2, types... tail) {
    	if (task1.isIdle()) {
    		if (task2.isIdle()) {
    			return sleepUntilTasks(tail...);
    		} else {
    			return sleepUntilTasks(task2, tail...);
    		}
    	} else {
    		if (task2.isIdle()) {
    			return sleepUntilTasks(task1, tail...);
    		} else {
    			// both tasks are non-idle, let's compare them.
    	    	auto mode = (task1.getMaxSleepMode() > task2.getMaxSleepMode()) ? task2.getMaxSleepMode() : task1.getMaxSleepMode();
    	    	auto time1 = toMillisOn<rt_t>(task1.timeLeft());
    	    	auto time2 = toMillisOn<rt_t>(task2.timeLeft());
    	    	auto time = (time1 > time2) ? time2 : time1;
    	    	return sleepUntilTasks(::TaskStateBusyFor(time, mode), tail...);
    		}
    	}
    }

    /**
     * Attempts to sleep (power down) for at most the given time.
     * A hardware or pin change interrupt can cause premature wake-up.
     * Sleeping is performed by sleeping multiple consecutive watchdog timer intervals. If an interrupt occurs
     * during a watchdog timer interval, the function guesses that 1/2 of that interval's time has elapsed. This causes
     * real-time keeping to go out of sync by, on average 1/2 of the interval time. Set [maxGranularity] to the
     * highest interval that's acceptable (higher values means less power used).
     *
     * Returns whether any hardware interrupts have interrupted sleep.
     *
     * @param time Time to sleep, subclass of RuntimeTimeUnit. Must be >16ms, and granularity
     * is only about 16ms.
     * @param mode Sleep mode to enter while sleeping
     * @param maxGranularity Coursest granularity for individual sleep steps. Courser granularity will give higher
     * power savings, but less time precision.
     */
    template <typename time_t>
    bool sleepFor(time_t time, SleepMode mode, SleepGranularity maxGranularity) {
        return sleepFor(toMillisOn<rt_t>(time), mode, maxGranularity);
    }

    template <typename time_t>
    bool sleepFor(time_t time, SleepMode mode) {
        return sleepFor(time, mode, SleepGranularity::_8000ms);
    }

    bool sleepFor(Milliseconds<> ms, SleepMode mode, SleepGranularity maxGranularity) {
        return doSleepFor(ms, mode, maxGranularity);
    }
};
}

template <typename rt_t>
Impl::Power<rt_t> Power(rt_t &rt) {
    return Impl::Power<rt_t>(rt);
}

}
}



#endif /* HAL_ATMEL_POWER_HPP_ */
