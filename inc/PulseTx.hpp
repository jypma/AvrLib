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

template <int fifoSize=16>
class SimplePulseTxSource {
    Fifo<fifoSize> fifo;
    bool idleHigh;
public:
    SimplePulseTxSource(bool _idleHigh): idleHigh(_idleHigh) {}

    Pulse getNextPulse() {
        Pulse pulse;
        return (fifo.in() >> pulse) ? pulse : Pulse::empty();
    }

    bool isHighOnIdle() {
        return idleHigh;
    }

    void append(Pulse const &pulse) {
        fifo.out() << pulse;
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
template <typename comparator_t, typename target_t, typename source_t>
class PulseTx {
    typedef PulseTx<comparator_t,target_t,source_t> This;
    typedef typename comparator_t::value_t count_t;
protected:
    comparator_t * const comparator;
    target_t * const target;
    source_t * const src;
    count_t lastPulseEnd = 0;
    Pulse pulse = Pulse::empty();

    void onComparator() {
        pulse = src->getNextPulse();
        if (pulse.isDefined()) {
            target->onIntermediateTransition(pulse.isHigh());
            lastPulseEnd += pulse.getDuration();
            comparator->setTarget(lastPulseEnd);
        } else {
            target->onFinalTransition(src->isHighOnIdle());
            comparator->interruptOff();
        }
    }
    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    PulseTx(comparator_t &_comparator, target_t &_target, source_t &_source): comparator(&_comparator), target(&_target), src(&_source) {
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
            target->onInitialTransition(pulse.isHigh());
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


template <typename pin_t>
struct HardwarePulseTxTarget {
    pin_t &pin;
    inline HardwarePulseTxTarget(pin_t &_pin): pin(_pin) {
        // contrary to the datasheet, setting DDR on the output pin actually seems to disconnect the timer comparator.
        pin.configureAsInputWithoutPullup();
    }

    inline void onInitialTransition(bool high) {
        pin.setHigh(high);
        pin.timerComparator().output(high ? NonPWMOutputMode::low_on_match : NonPWMOutputMode::high_on_match);
    }
    inline void onIntermediateTransition(bool high) {
        pin.timerComparator().output(high ? NonPWMOutputMode::low_on_match : NonPWMOutputMode::high_on_match);
    }
    inline void onFinalTransition(bool high) {
        pin.setHigh(high);
        pin.timerComparator().output(NonPWMOutputMode::disconnected);
    }
};

template <typename pin_t, typename source_t = SimplePulseTxSource<>>
class HardwarePulseTx: public PulseTx<typename pin_t::comparator_t, HardwarePulseTxTarget<pin_t>, source_t> {
    HardwarePulseTxTarget<pin_t> target;
public:
    HardwarePulseTx(pin_t &pin, source_t &source = source_t::create()):
        PulseTx<typename pin_t::comparator_t, HardwarePulseTxTarget<pin_t>, source_t>(pin.timerComparator(), target, source), target(pin) {}
};
template <typename pin_t, typename source_t>
inline HardwarePulseTx<pin_t, source_t> hardwarePulseTx(pin_t &pin, source_t &source) {
    return HardwarePulseTx<pin_t, source_t>(pin, source);
}

template <typename target_t>
struct SoftwarePulseTxTarget {
    target_t &target;
    inline SoftwarePulseTxTarget(target_t &_target): target(_target) {}

    inline void onInitialTransition(bool high) {
        target.setHigh(high);
    }
    inline void onIntermediateTransition(bool high) {
        target.setHigh(high);
    }
    inline void onFinalTransition(bool high) {
        target.setHigh(high);
    }
};

template <typename comparator_t, typename target_t, typename source_t>
class SoftwarePulseTx: public PulseTx<comparator_t, SoftwarePulseTxTarget<target_t>, source_t> {
    SoftwarePulseTxTarget<target_t> target;
public:
    SoftwarePulseTx(comparator_t &_comparator, target_t &_target, source_t &_source):
        PulseTx<comparator_t, SoftwarePulseTxTarget<target_t>, source_t>(_comparator, target, _source), target(_target) {}
};

template <typename comparator_t, typename target_t, typename source_t>
inline SoftwarePulseTx<comparator_t, target_t, source_t> softwarePulseTx(comparator_t &_comparator, target_t &_target, source_t &_source) {
    return SoftwarePulseTx<comparator_t, target_t, source_t>(_comparator, _target, _source);
};

#endif /* PULSETX_HPP_ */
