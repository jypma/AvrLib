/*
 * RealTimer.hpp
 *
 *  Created on: Jan 4, 2015
 *      Author: jan
 */

#ifndef REALTIMER_HPP_
#define REALTIMER_HPP_

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Time/Prescaled.hpp"
#include "Time/Units.hpp"
#include "AtomicScope.hpp"
#include <gcc_limits.h>
#include <gcc_type_traits.h>
#include <util/atomic.h>
#include <avr/sleep.h>

namespace HAL { namespace Atmel { namespace Impl { template <typename> class Power; }}}

namespace Time {

namespace Impl {

inline void noop() {

}

}

template <typename rt_t, typename value>
struct Overflow {
    static constexpr bool countsLargerThanUint31 = toCountsOn<rt_t,value>().value >= 0xFFFFFFF;
    static constexpr bool countsLargerThanUint32 = !toCountsOn<rt_t,value>().is_uint32;
};

using namespace HAL::Atmel::InterruptHandlers;

template<typename timer_t, uint32_t initialTicks = 0, void (*wait)() = Impl::noop>
class RealTimer: public Time::Prescaled<typename timer_t::value_t, typename timer_t::prescaler_t, timer_t::prescaler> {
    typedef RealTimer<timer_t,initialTicks,wait> This;

    template <typename> friend class HAL::Atmel::Impl::Power;

    volatile uint32_t _ticks = initialTicks;
    timer_t *timer;

    void onTimerOverflow() {
       _ticks++;
    }

    template <typename time_t>
    void haveSlept(time_t time) {
        uint32_t delta = toTicksOn<This>(time).getValue();
        AtomicScope _;
        _ticks += delta;
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


    void delayCounts(uint32_t countsDelay) const {
        auto startTime = counts().getValue();
        if (uint32_t(0xFFFFFFFF) - startTime < countsDelay) {
            // we expect a integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (counts().getValue() > 1) {
                wait();
            }
        }
        uint32_t end = startTime + countsDelay;
        while (counts().getValue() < end) {
            wait();
        }
    }

public:
    typedef On<This, typename timer_t::INT, &This::onTimerOverflow> Handlers;

    RealTimer(timer_t &_timer): timer(&_timer) {
        timer->interruptOnOverflowOn();
    }

    /**
     * Returns a 32-bit value that increments with every timer overflow.
     */
    Ticks<> ticks() const {
        AtomicScope _;
        return _ticks;
    }

    /**
     * Returns a 32-bit value that increments with every timer increment.
     */
    Counts<> counts() const {
        AtomicScope _;
        return (_ticks << timer_t::maximumPower2) | timer->getValue();
    }

    Microseconds<> micros() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)

        return (((((uint64_t)_ticks) << timer_t::prescalerPower2) + timer->getValue()) / 16) << timer_t::maximumPower2;
    }

    Milliseconds<> millis() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)
        // divide by 1000 to get milliseconds

        return ((((((uint64_t)_ticks) << timer_t::prescalerPower2) + timer->getValue()) / 16) << timer_t::maximumPower2) / 1000;
    }

    template <typename duration_t>
    typename std::enable_if<Overflow<This, duration_t>::countsLargerThanUint31>::type delay(const duration_t) {
        delayTicks(toTicksOn<timer_t,duration_t>());
    }

    template <typename duration_t>
    typename std::enable_if<!Overflow<This, duration_t>::countsLargerThanUint31>::type delay(const duration_t) {
        delayCounts(toCountsOn<timer_t,duration_t>());
    }
};

template<typename timer_t, uint32_t initialTicks = 0, void (*wait)() = Impl::noop>
RealTimer<timer_t,initialTicks,wait> realTimer(timer_t &timer) {
    return RealTimer<timer_t,initialTicks,wait>(timer);
}

class AbstractPeriodic {
    volatile uint32_t next;
    bool waitForOverflow = false;
protected:
    uint32_t getNext() const { return next; }
    void calculateNextCounts(uint32_t startTime, uint32_t delay);
    bool isNow(uint32_t currentTime, uint32_t delay);
    uint32_t getTimeLeft(uint32_t currentTime) const;
};

template <typename rt_t, typename value, typename check = void>
class Periodic: public AbstractPeriodic {
    static constexpr uint32_t delay = toCountsOn<rt_t, value>();
    static_assert(delay < 0xFFFFFFF, "Delay must fit in 2^31 in order to cope with timer inter wraparound");

    rt_t *rt;
public:
    Periodic(rt_t &_rt): rt(&_rt) {
        calculateNextCounts(rt->counts(), delay);
    }

    bool isNow() {
        return AbstractPeriodic::isNow(rt->counts(), delay);
    }

    Counts<> timeLeft() const {
        return Counts<>(getTimeLeft(rt->counts()));
    }
};

template <typename rt_t, typename value>
class Periodic<rt_t, value, typename std::enable_if<Overflow<rt_t, value>::countsLargerThanUint31>::type>: public AbstractPeriodic {
    static constexpr uint32_t delay = toTicksOn<rt_t, value>();
    static_assert(delay < 0xFFFFFFF, "Delay must fit in 2^31 in order to cope with timer inter wraparound");

    rt_t *rt;
public:
    Periodic(rt_t &_rt): rt(&_rt) {
        calculateNextCounts(rt->ticks(), delay);
    }

    bool isNow() {
        return AbstractPeriodic::isNow(rt->ticks(), delay);
    }

    Ticks<> timeLeft() const {
        return Ticks<>(getTimeLeft(rt->ticks()));
    }
};

template <typename rt_t, typename value_t>
Periodic<rt_t,value_t> periodic(rt_t &rt, value_t value) {
    return Periodic<rt_t,value_t>(rt);
}

class AbstractDeadline {
    volatile uint32_t next;
    bool waitForOverflow = false;
protected:
    volatile bool elapsed;
    AbstractDeadline(bool _elapsed): elapsed(_elapsed) {}

    uint32_t getNext() const {
        return next;
    }
    bool isNow(uint32_t currentTime);
    void calculateNext(uint32_t startTime, uint32_t delay);
    uint32_t getTimeLeft(uint32_t currentTime) const;
public:
    /**
     * Returns whether the deadline has already elapsed, i.e. is not scheduled.
     */
    bool isElapsed() {
        return elapsed;
    }

    /**
     * Returns whether the deadline is scheduled to elapsed some time in the future.
     */
    bool isScheduled() {
        return !elapsed;
    }

    /**
     * Cancels any scheduled elapsing of this deadline.
     */
    void cancel() {
        elapsed = true;
    }
};

template <typename rt_t, typename value, typename check = void>
class Deadline: public AbstractDeadline {
    rt_t *rt;
public:
    static constexpr uint32_t delay = toCountsOn<rt_t, value>();
    static_assert(delay < 0xFFFFFFF, "Delay must fit in 2^31 in order to cope with timer integer wraparound");
public:
    Deadline(rt_t &_rt): AbstractDeadline(false), rt(&_rt) {
        calculateNext(rt->counts(), delay);
    }

    bool isNow() {
        return AbstractDeadline::isNow(rt->counts());
    }

    void schedule() {
        AtomicScope _;
        calculateNext(rt->counts(), delay);
        elapsed = false;
    }

    Counts<> timeLeft() const {
        return getTimeLeft(rt->counts());
    }
};

template <typename rt_t, typename value>
class Deadline<rt_t, value, typename std::enable_if<Overflow<rt_t, value>::countsLargerThanUint31>::type>: public AbstractDeadline {
    rt_t *rt;
protected:
    static constexpr uint32_t delay = toTicksOn<rt_t, value>();
    static_assert(delay < 0xFFFFFFF, "Delay must fit in 2^31 in order to cope with timer integer wraparound");
public:
    Deadline(rt_t &_rt): AbstractDeadline(false), rt(&_rt) {
        calculateNext(rt->ticks(), delay);
    }

    bool isNow() {
        return AbstractDeadline::isNow(rt->ticks());
    }

    void schedule() {
        AtomicScope _;
        calculateNext(rt->ticks(), delay);
        elapsed = false;
    }

    Ticks<> timeLeft() const {
        return getTimeLeft(rt->ticks());
    }
};


template <typename rt_t>
class VariableDeadline: public AbstractDeadline {
    rt_t *rt;
protected:
    uint32_t getNow() {
        return rt->counts();
    }
public:
    VariableDeadline(rt_t &_rt): AbstractDeadline(true), rt(&_rt) {}

    bool isNow() {
        return AbstractDeadline::isNow(rt->counts());
    }

    template <typename value>
    void schedule() {
        static constexpr uint32_t delay = toCountsOn<rt_t, value>();
        static_assert(delay < 0xFFFFFFF, "Delay must fit in 2^31 in order to cope with timer inter wraparound");

        AtomicScope _;
        calculateNext(rt->counts(), delay);
        elapsed = false;
    }

    template <typename value>
    void schedule(value v) {
        schedule<value>();
    }

    Counts<> timeLeft() const {
        return getTimeLeft(rt->counts());
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

class AnimatorEvent {
    /**
     * Whether more changes to [value] are to be expected within the current animation
     */
    bool animating;
    /**
     * Whether [value] changed since the previous returned event
     */
    bool changed;
    /**
     * The current animation value
     */
    uint16_t value;
public:
    AnimatorEvent(bool _animating, bool _changed, uint16_t _value): animating(_animating), changed(_changed), value(_value) {}

    uint16_t getValue() const {
        return value;
    }

    bool isIdle() const {
        return !animating;
    }

    bool isAnimating() const {
        return animating;
    }

    bool isChanged() const {
        return changed;
    }

    bool operator== (const AnimatorEvent that) const {
        return animating == that.animating && changed == that.changed && value == that.value;
    }
};

/**
 * The interpolation method to use in Animator steps.
 */
enum class AnimatorInterpolation: uint8_t {
    /** Interpolate linearly between in and out point */
    LINEAR,
    /** Interpolate using t^2, starting with small steps, taking bigger steps as the out point approaches */
    EASE_IN,
    /** Interpolate using (1-t)^2, starting with big steps, taking smaller steps as the out point approaches */
    EASE_OUT,
    /** Resolves to EASE_IN if animating from low to high values, or EASE_OUT otherwise */
    EASE_UP,
    /** Resolves to EASE_OUT if animating from low to high values, or EASE_IN otherwise */
    EASE_DOWN
};

template <typename rt_t>
class Animator: protected VariableDeadline<rt_t> {
    typedef VariableDeadline<rt_t> Super;
    typedef Animator<rt_t> This;

    uint16_t start = 0;
    uint16_t span = 0;
    bool backwards = false;
    uint16_t delay = 0;
    uint32_t startTime = 0;
    bool done = true;
    bool haveSentComplete = true;
    bool atStart = false;
    AnimatorInterpolation mode = AnimatorInterpolation::LINEAR;
    uint16_t lastValue = 0;

    uint16_t getDelta() {
        const uint32_t progress = int32_t(Super::getNow() - startTime);

        switch(mode) {
        case AnimatorInterpolation::LINEAR:
            return progress * span / delay;
        case AnimatorInterpolation::EASE_IN:
            return float(progress) * progress * span / delay / delay;
            //std::cout << "progress= " << int(progress) << " span=" << int(span) << " delay=" << int(delay) << std::endl;
            //if (progress < span) {
            //    return progress * span / delay * progress / delay;
            //} else {
            //    return progress * progress * span / delay / delay;
            // }
        case AnimatorInterpolation::EASE_OUT: {
            const uint32_t factor = (delay - progress);
            return span - float(factor) * factor * span / delay / delay;
            /*
            if (factor < span) {
                return span - (factor * span / delay * factor / delay);
            } else {
                return span - (factor * factor * span / delay / delay);
            }
            */
        }
        default: // shouldn't occur
            return 0;
        }

        return 0;
    }
public:
    Animator(rt_t &_rt): Super(_rt) {}

    /**
     * Starts the next "to" call from the given [start] value. Any animation in progress is stopped.
     */
    This &from(uint16_t _start) {
        AtomicScope _;

        start = _start;
        lastValue = _start;
        startTime = Super::getNow();
        done = true;
        haveSentComplete = true;
        atStart = true;

        return *this;
    }

    template <typename duration_t>
    void to(const uint16_t end, duration_t d) {
        to (end, d, AnimatorInterpolation::LINEAR);
    }

    template <typename duration_t>
    void to(const uint16_t end, duration_t d, AnimatorInterpolation _mode) {
        AtomicScope _;

        mode = _mode;
        start = lastValue;
        backwards = end < start;
        if (backwards) {
            span = start - end;
        } else {
            span = end - start;
        }
        if (_mode == AnimatorInterpolation::EASE_UP) {
            mode = (backwards) ? AnimatorInterpolation::EASE_OUT : AnimatorInterpolation::EASE_IN;
        } else if (_mode == AnimatorInterpolation::EASE_DOWN) {
            mode = (backwards) ? AnimatorInterpolation::EASE_IN : AnimatorInterpolation::EASE_OUT;
        }
        delay = toCountsOn<rt_t>(d);
        startTime = Super::getNow();
        if (atStart) {
            // haven't invoked nextEvent() since from(), so we should report the start value.
            lastValue = start - 1; // force trigger an UPDATED output
        }
        done = false;
        haveSentComplete = false;
        atStart = false;
        Super::schedule(d);
    }

    AnimatorEvent nextEvent() {
        AtomicScope _;

        if (Super::isNow()) {
            done = true;
        }

        if (atStart) {
            atStart = false;
            lastValue = start;
            return { !done, true, lastValue };
        } else if (done) {
            if (haveSentComplete) {
                return { false, false, lastValue };
            } else {
                haveSentComplete = true;
                lastValue = (backwards) ? start - span : start + span;
                return { false, true, lastValue };
            }
        } else {
            const uint16_t value = (backwards) ? start - getDelta() : start + getDelta();

            if (value != lastValue) {
                lastValue = value;
                return { true, true, lastValue };
            } else {
                return { true, false, lastValue };
            }
        }
    }
};

template <typename rt_t>
Animator<rt_t> animator(rt_t &rt) {
    return Animator<rt_t>(rt);
}

}

#endif /* REALTIMER_HPP_ */
