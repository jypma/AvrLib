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
#include "Streams/Streamable.hpp"

namespace Serial {

using namespace Streams;

class Pulse: public Streamable<Pulse> {
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

    typedef Format<
        Scalar<bool, &Pulse::high>,
        Scalar<uint16_t, &Pulse::duration>
    > Proto;
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

    inline void onIdle(bool high) {

    }

    inline void onInitialTransition(bool high) {
        //pin->setHigh(high);   // has no effect, because of the setOutput() later on.

        //pin->timerComparator().setOutput(high ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
        //pin->timerComparator().applyOutput();

        //pin->timerComparator().setOutput()

        // We postpone the initial transition, and let the timer comparator do it.
        // We need to delay the intial transition, and have that one be from the comparator as well.

        /*
         * PulseTx.sendFromSource()
         *
         *    0:  getNextPulse:   low, 200
         *      onInitialPulse(Pulse)
         *        setLow()
         *        lastDuration = 200
         *    1:  getNextPulse:   high, 100
         *      onIntermediatePulse(Pulse)
         *        comparator.after (lastDuration, high)
         *        lastDuration = 100
         *    N:
         *      onPulsesComplete(high)
         *        comparator.after (lastDuration, high)
         *
         *   * OCRA  ->
         *     getNextPulse:   low, 200
         *       setLow() <-- was too late
         */
        pin->timerComparator().setOutput(high ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
    }
    inline void onIntermediateTransition(bool high) {
        pin->timerComparator().setOutput(high ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
    }
    inline void onFinalTransition(bool high) {
        pin->setHigh(high);
        pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
    }
};

/**
 * Sends out individual pulses. The object must stay in scope for the duration of the pulse(s)!
 *
 * comparator_t comparator    : The NonPWMTimerComparator to use as clock source
 * target_t target            : Target, or Pin, to apply output on. Must have setHigh(bool).
 * source_t source            : Provides further pulses, hasNextPulse() and getNextPulseDuration().
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

/**
 * Sends out individual pulses to a pin that supports hardware PWM through a timer comparator.
 *
 * pin_t pin       : The target pin to send the waveform to
 * source_t source : Provides the stream of pulses to send
 */
template <typename pin_t, typename source_t>
class ComparatorPinPulseTx {
    typedef ComparatorPinPulseTx<pin_t,source_t> This;
    typedef typename pin_t::value_t count_t;
protected:
    pin_t * const pin;
    source_t * const src;
    count_t lastPulseEnd = 0;
    count_t nextPulseDuration = 0;
    Pulse pulse = Pulse::empty();

    void onComparator() {
        pulse = src->getNextPulse();
        lastPulseEnd += nextPulseDuration;
        pin->timerComparator().setTarget(lastPulseEnd);
        if (pulse.isDefined()) {
            pin->timerComparator().setOutput(pulse.isHigh() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
            nextPulseDuration = pulse.getDuration();
        } else {
            pin->timerComparator().setOutput(src->isHighOnIdle() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
            pin->timerComparator().interruptOff();
        }
    }
    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    ComparatorPinPulseTx(pin_t &_pin, source_t &_source): pin(&_pin), src(&_source) {
        pin->configureAsOutput();
        pin->timerComparator().interruptOff();
        pin->timerComparator().setOutput(src->isHighOnIdle() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
        pin->timerComparator().interrupt().attach(onComparatorInt);
    }

    ~ComparatorPinPulseTx() {
        pin->timerComparator().interrupt().detach();
        pin->setHigh(src->isHighOnIdle());
    }

    /** Checks the source for new pulses and starts sending them. */
    void sendFromSource() {
        AtomicScope _;

        if (pulse.isDefined()) return;
        pulse = src->getNextPulse();
        if (pulse.isDefined()) {
            Pulse nextPulse = src->getNextPulse();
            lastPulseEnd = pin->timerComparator().getValue() + pulse.getDuration();
            pin->setHigh(pulse.isHigh());
            pin->timerComparator().setTarget(lastPulseEnd);
            pulse = nextPulse;
            if (pulse.isDefined()) {
                nextPulseDuration = pulse.getDuration();
                pin->timerComparator().interruptOn();
                pin->timerComparator().setOutput(pulse.isHigh() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
            } else {
                pin->timerComparator().setOutput(src->isHighOnIdle() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
            }
        }
    }

    inline typename pin_t::comparator_t &timerComparator() {
        return pin->timerComparator();
    }

    inline source_t &source() {
        return *src;
    }

    inline bool isSending() {
        return pulse.isDefined();
    }
};

template <typename pin_t, typename source_t>
inline ComparatorPinPulseTx<pin_t, source_t> pulseTx(pin_t &pin, source_t &source) {
    return ComparatorPinPulseTx<pin_t, source_t>(pin, source);
}

}

#endif /* PULSETX_HPP_ */
