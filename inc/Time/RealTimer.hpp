/*
 * RealTimer.hpp
 *
 *  Created on: Jan 4, 2015
 *      Author: jan
 */

#ifndef REALTIMER_HPP_
#define REALTIMER_HPP_

#include "HAL/Atmel/InterruptVectors.hpp"
#include "Time/Prescaled.hpp"
#include "Time/Units.hpp"
#include "AtomicScope.hpp"
#include <gcc_limits.h>
#include <gcc_type_traits.h>

inline void noop() {

}

namespace Time {

template<typename timer_t, uint32_t initialTicks = 0, void (*wait)() = noop>
class RealTimer: public Time::Prescaled<typename timer_t::value_t, typename timer_t::prescaler_t, timer_t::prescaler> {
    typedef RealTimer<timer_t,initialTicks,wait> This;

    volatile uint32_t _ticks = initialTicks;
    timer_t *timer;

    void onTimerOverflow() {
       _ticks++;
    }

public:
    RealTimer(timer_t &_timer): timer(&_timer) {
        timer->interruptOnOverflowOn();
    }

    /**
     * Returns a 32-bit value that increments with every timer overflow.
     */
    uint32_t ticks() const {
        AtomicScope _;
        return _ticks;
    }

    /**
     * Returns a 32-bit value that increments with every timer increment.
     */
    uint32_t counts() const {
        AtomicScope _;
        return (_ticks << timer_t::maximumPower2) | timer->getValue();
    }

    uint64_t micros() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)

        return ((((uint64_t)_ticks) << timer_t::prescalerPower2) / 16) << timer_t::maximumPower2;
    }

    uint64_t millis() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)
        // divide by 1000 to get milliseconds

        return (((((uint64_t)_ticks) << timer_t::prescalerPower2) / 16) << timer_t::maximumPower2) / 1000;
    }

    void delayTicks(uint32_t ticksDelay) const {
        auto startTime = ticks();
        if (uint32_t(0xFFFFFFFF) - startTime < ticksDelay) {
            // we expect a integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (ticks() > 1) {
                wait();
            }
        }
        uint32_t end = startTime + ticksDelay;
        while (ticks() < end) {
            wait();
        }
    }

    //TODO increase accuracy by delaying ticks first, then counts for the rest.
    template <typename duration_t>
    void delay(const duration_t) {
        delayTicks(toTicksOn<timer_t,duration_t>());
    }

    INTERRUPT_HANDLER1(typename timer_t::INT, onTimerOverflow);
};

template<typename timer_t, uint32_t initialTicks = 0, void (*wait)() = noop>
RealTimer<timer_t,initialTicks,wait> realTimer(timer_t &timer) {
    return RealTimer<timer_t,initialTicks,wait>(timer);
}

template <typename rt_t, typename value>
struct Overflow {
    static constexpr bool largerThanUint32 = !toCountsOn<rt_t,value>().is_uint32;
};


template <typename rt_t, typename value, typename check = void>
class Periodic {
    static constexpr uint32_t delay = toCountsOn<rt_t, value>();

protected:
    volatile uint32_t next;
    rt_t *rt;

    inline uint32_t currentTime() {
        return rt->counts();
    }

    void calculateNextCounts(uint32_t startTime) {
        if (uint32_t(0xFFFFFFFF) - startTime < delay) {
            // we expect an integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (currentTime() > 5) ;
        }
        next = startTime + delay;
    }
public:
    Periodic(rt_t &_rt): rt(&_rt) {
        calculateNextCounts(currentTime());
    }

    bool isNow() {
        if (currentTime() >= next) {
            calculateNextCounts(next);
            return true;
        } else {
            return false;
        }
    }
};

template <typename rt_t, typename value>
class Periodic<rt_t, value, typename std::enable_if<Overflow<rt_t, value>::largerThanUint32>::type> {
    static constexpr uint32_t delay = toTicksOn<rt_t, value>();

protected:
    volatile uint32_t next;
    rt_t *rt;

    inline uint32_t currentTime() {
        return rt->ticks();
    }

    void calculateNextCounts(uint32_t startTime) {
        if (uint32_t(0xFFFFFFFF) - startTime < delay) {
            // we expect an integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (currentTime() > 5) ;
        }
        next = startTime + delay;
    }
public:
    Periodic(rt_t &_rt): rt(&_rt) {
        calculateNextCounts(currentTime());
    }

    bool isNow() {
        if (currentTime() >= next) {
            calculateNextCounts(next);
            return true;
        } else {
            return false;
        }
    }
};

template <typename rt_t, typename value_t>
Periodic<rt_t,value_t> periodic(rt_t &rt, value_t value) {
    return Periodic<rt_t,value_t>(rt);
}

template <typename rt_t, typename value>
class Deadline: public Periodic<rt_t, value> {
    typedef Periodic<rt_t, value> Super;
    using Super::currentTime;
    using Super::next;

    volatile bool elapsed = false;
public:
    Deadline(rt_t &_rt): Super(_rt) {}

    bool isNow() {
        if (!elapsed) {
            if (currentTime() >= next) {
                elapsed = true;
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    void reset() {
        Super::calculateNextCounts(currentTime());
        elapsed = false;
    }
};

template <typename rt_t>
class VariableDeadline {
    rt_t *rt;
    uint32_t next = 0;
    volatile bool elapsed = true;

public:
    VariableDeadline(rt_t &_rt): rt(&_rt) {}

    bool isNow() {
        if (!elapsed) {
            if (rt->counts() >= next) {
                elapsed = true;
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    template <typename value>
    void reset(value v) {
        constexpr uint32_t delay = toCountsOn<rt_t>(v);
        uint32_t startTime = rt->counts();
        if (uint32_t(0xFFFFFFFF) - startTime < delay) {
            // we expect an integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (rt->counts() > 5) ;
        }
        next = startTime + delay;
        elapsed = false;
    }
};

template <typename rt_t, typename value_t>
Deadline<rt_t,value_t> deadline(rt_t &rt, value_t value) {
    return Deadline<rt_t,value_t>(rt);
}

template <typename rt_t>
VariableDeadline<rt_t> deadline(rt_t &rt) {
    return VariableDeadline<rt_t>(rt);
}

}

#endif /* REALTIMER_HPP_ */
