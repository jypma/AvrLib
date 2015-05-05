/*
 * PulseTx.hpp
 *
 *  Created on: Apr 17, 2015
 *      Author: jan
 */

#ifndef PULSETX_HPP_
#define PULSETX_HPP_

#include <stdint.h>
#include "InterruptHandler.hpp"
#include "Timer.hpp"
#include "Fifo.hpp"

class Pulse {
    bool high;
    uint16_t duration;
public:
    constexpr Pulse(): high(false), duration(0) {}
    constexpr Pulse(const bool _high, const uint16_t _duration): high(_high), duration(_duration) {}

    uint16_t getDuration() const {
        return duration;
    }
    bool isHigh() const {
        return high;
    }
    bool isEmpty() const {
        return duration == 0;
    }
    bool isDefined() const {
        return duration != 0;
    }

    static constexpr Pulse empty() {
        return Pulse();
    }

    static void write(Writer &out, const Pulse &pulse) {
        out << pulse.high << pulse.duration;
    }
    static void read(Reader &in, Pulse &pulse) {
        in >> pulse.high >> pulse.duration;
    }
};

template <typename Value, bool high>
struct PulseIn {
    template <typename prescaled_t>
    static constexpr Pulse on() {
        return Pulse(high, Value::template toCounts<prescaled_t>());
    }
};

template <typename Value>
PulseIn<Value, true> constexpr highPulse(Value duration) {
    return PulseIn<Value, true>();
}

template <typename Value>
PulseIn<Value, false> constexpr lowPulse(Value duration) {
    return PulseIn<Value, false>();
}

template <typename target_t>
class PulseTxCallbackTarget {
    target_t *target;
public:
    inline PulseTxCallbackTarget(target_t &_target): target(&_target) {}

    inline void onInitialTransition(bool high) {
        target->setHigh(high);
    }
    inline void onIntermediateTransition(bool high) {
        target->setHigh(high);
    }
    inline void onFinalTransition(bool high) {
        target->setHigh(high);
    }
};

template <typename pin_t>
class PulseTxComparatorPinTarget {
    pin_t *pin;
public:
    inline PulseTxComparatorPinTarget(pin_t &_pin): pin(&_pin) {
        pin->configureAsOutput();
    }

    inline void onInitialTransition(bool high) {
        pin->setHigh(high);
        pin->timerComparator().output(high ? NonPWMOutputMode::low_on_match : NonPWMOutputMode::high_on_match);
    }
    inline void onIntermediateTransition(bool high) {
        pin->timerComparator().output(high ? NonPWMOutputMode::low_on_match : NonPWMOutputMode::high_on_match);
    }
    inline void onFinalTransition(bool high) {
        pin->setHigh(high);
        pin->timerComparator().output(NonPWMOutputMode::disconnected);
    }
};

/**
 * Sends out individual pulses. The object must stay in scope for the duration of the pulse(s)!
 *
 * comparator_t comparator    : The NonPWMTimerComparator to use as clock source
 * target_t target            : Target, or Pin, to apply output on. Must have setHigh(bool).
 * source_t source            : Provides further pulses, hasNextPulse() and getNextPulseDuration(). Optional, in which case it'll just be a single pulse.
 *                              TODO template specialization when target is in fact the output pin of comparator_t.
 */
template <typename comparator_t, typename target_t, typename target_wrapper_t, typename source_t>
class PulseTx {
    typedef PulseTx<comparator_t,target_t,target_wrapper_t,source_t> This;
    typedef typename comparator_t::value_t count_t;
protected:
    comparator_t * const comparator;
    target_wrapper_t target;
    source_t * const src;
    count_t lastPulseEnd = 0;
    Pulse pulse = Pulse::empty();

    void onComparator() {
        pulse = src->getNextPulse();
        if (pulse.isDefined()) {
            target.onIntermediateTransition(pulse.isHigh());
            lastPulseEnd += pulse.getDuration();
            comparator->setTarget(lastPulseEnd);
        } else {
            target.onFinalTransition(src->isHighOnIdle());
            comparator->interruptOff();
        }
    }
    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    PulseTx(comparator_t &_comparator, target_t &_target, source_t &_source): comparator(&_comparator), target(_target), src(&_source) {
        comparator->interrupt().attach(onComparatorInt);
    }

    ~PulseTx() {
        comparator->interrupt().detach();
    }

    /** Checks the source for new pulses and starts sending them. */
    void sendFromSource() {
        AtomicScope _;

        if (pulse.isDefined()) return;
        pulse = src->getNextPulse();
        if (pulse.isDefined()) {
            lastPulseEnd = comparator->getValue() + pulse.getDuration();
            target.onInitialTransition(pulse.isHigh());
            comparator->setTarget(lastPulseEnd);
            comparator->interruptOn();
        }
    }

    inline comparator_t &timerComparator() {
        return *comparator;
    }

    inline source_t &source() {
        return *src;
    }

    inline bool isSending() {
        return pulse.isDefined();
    }
};

template <typename comparator_t, typename target_t, typename source_t>
using CallbackPulseTx = PulseTx<comparator_t, target_t, PulseTxCallbackTarget<target_t>, source_t>;

template <typename comparator_t, typename target_t, typename source_t>
inline CallbackPulseTx<comparator_t, target_t, source_t> pulseTx(comparator_t &comparator, target_t &target, source_t &source) {
    return CallbackPulseTx<comparator_t, target_t, source_t>(comparator, target, source);
}

template <typename pin_t, typename source_t>
using ComparatorPinPulseTx = PulseTx<typename pin_t::comparator_t, pin_t, PulseTxComparatorPinTarget<pin_t>, source_t>;

template <typename pin_t, typename source_t>
inline ComparatorPinPulseTx<pin_t, source_t> pulseTx(pin_t &pin, source_t &source) {
    return ComparatorPinPulseTx<pin_t, source_t>(pin.timerComparator(), pin, source);
}


#endif /* PULSETX_HPP_ */
