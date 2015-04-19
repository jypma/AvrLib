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

template <typename count_t>
struct PulseTxNoSource {
    inline count_t getNextPulseDuration() { return 0; }
};

/**
 * Sends out individual pulses. The object must stay in scope for the duration of the pulse(s)!
 *
 * comparator_t comparator    : The NonPWMTimerComparator to use as clock source
 * target_t target            : Target, or Pin, to apply output on. Must have setHigh(bool).
 * source_t source            : Provides further pulses, hasNextPulse() and getNextPulseDuration(). Optional, in which case it'll just be a single pulse.
 *                              TODO template specialization when target is in fact the output pin of comparator_t.
 */
template <typename comparator_t, typename target_t, typename source_t = PulseTxNoSource<typename comparator_t::value_t>>
class PulseTx {
    typedef PulseTx<comparator_t,target_t,source_t> This;
    typedef typename comparator_t::value_t count_t;
protected:
    comparator_t * const comparator;
    target_t * const target;
    source_t * const source;
    count_t lastPulseEnd = 0;
    bool high = false;

    void onComparator() {
        high = !high;
        count_t nextDuration = (source != nullptr) ? source->getNextPulseDuration() : 0;
        if (nextDuration > 0) {
            target->onIntermediateTransition(high);
            lastPulseEnd += nextDuration;
            comparator->setTarget(lastPulseEnd);
        } else {
            target->onFinalTransition(high);
            comparator->interruptOff();
        }
    }
    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    PulseTx(comparator_t &_comparator, target_t &_target, source_t &_source): comparator(&_comparator), target(&_target), source(&_source) {
        comparator->interrupt().attach(onComparatorInt);
    }

    PulseTx(comparator_t &_comparator, target_t &_target): comparator(&_comparator), target(&_target), source(nullptr) {
        comparator->interrupt().attach(onComparatorInt);
    }

    ~PulseTx() {
        comparator->interrupt().detach();
    }

    void pulse(bool _high, uint16_t duration) {
        lastPulseEnd = comparator->getValue() + duration;
        high = _high;
        target->onInitialTransition(high);
        comparator->setTarget(lastPulseEnd);
        comparator->interruptOn();
    }

    inline comparator_t &timerComparator() {
        return *comparator;
    }
};


template <typename pin_t>
struct HardwarePulseTxTarget {
    pin_t *pin;
    inline HardwarePulseTxTarget(pin_t *_pin): pin(_pin) {
        // contrary to the datasheet, setting DDR on the output pin actually seems to disconnect the timer comparator.
        pin->configureAsInputWithoutPullup();
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

template <typename pin_t, typename source_t = PulseTxNoSource<typename pin_t::comparator_t::value_t>>
class HardwarePulseTx: public PulseTx<typename pin_t::comparator_t, HardwarePulseTxTarget<pin_t>, source_t> {
    typedef PulseTx<typename pin_t::comparator_t, HardwarePulseTxTarget<pin_t>, source_t> Super;
    HardwarePulseTxTarget<pin_t> target;
public:
    HardwarePulseTx(pin_t &pin, source_t &source): Super(pin.timerComparator(), target, source), target(&pin) {}
    HardwarePulseTx(pin_t &pin): Super(pin.timerComparator(), target), target(&pin) {}
};

template <typename target_t>
struct SoftwarePulseTxTarget {
    target_t *target;
    inline SoftwarePulseTxTarget(target_t *_target): target(_target) {}

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

template <typename comparator_t, typename target_t, typename source_t = PulseTxNoSource<typename comparator_t::value_t>>
class SoftwarePulseTx: public PulseTx<comparator_t, SoftwarePulseTxTarget<target_t>, source_t> {
    typedef PulseTx<comparator_t, SoftwarePulseTxTarget<target_t>, source_t> Super;
    SoftwarePulseTxTarget<target_t> target;
public:
    SoftwarePulseTx(comparator_t &comparator, target_t &_target, source_t &source): Super(comparator, target, source), target(&_target) {}
    SoftwarePulseTx(comparator_t &comparator, target_t &_target): Super(comparator, target), target(&_target) {}
};



#endif /* PULSETX_HPP_ */
