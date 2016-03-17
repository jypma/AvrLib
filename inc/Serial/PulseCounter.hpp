/*
 * PulseCounter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jan
 */

#ifndef PULSECOUNTER_HPP_
#define PULSECOUNTER_HPP_

#include "HAL/Atmel/InterruptVectors.hpp"
#include "Fifo.hpp"
#include "HAL/Atmel/Timer.hpp"
#include "Serial/Pulse.hpp"
#include "Logging.hpp"

namespace Serial {

using namespace Streams;
using namespace Time;

/**
 * Counts up/down pulse lengths on a pin by using a timer. The longest reported length is
 * the timer's maximum value - 1.
 */
template <typename _comparator_t,
          typename pin_t,
          int fifo_length = 32>
class PulseCounter {
public:
    typedef PulseCounter<_comparator_t,pin_t,fifo_length> This;
    typedef _comparator_t comparator_t;
    typedef typename comparator_t::value_t count_t;
    PulseCounter (const PulseCounter<_comparator_t, pin_t, fifo_length> &) = default;

private:
    typedef Logging::Log<Loggers::Serial> log;

    Fifo<fifo_length> fifo;
    volatile count_t start;
    volatile bool wasEmptyPeriod = false;
    bool lastWriteWasHigh;

    _comparator_t *const comparator;
    pin_t *const pin;
    const count_t minimumLength;

    void onPinChanged() {
        lastWriteWasHigh = pin->isHigh();
        const count_t end = comparator->getValue();
        const count_t length = (end > start) ? end - start :
                               Counting<count_t>::maximum - (start - end);

        log::debug("onPinChanged start=%d end=%d high=%d", start, end, lastWriteWasHigh);
        fifo.write(length, uint8_t(pin->isHigh() ? 0 : 1));
        pin->interruptOff();
        comparator->setTarget(end + minimumLength);

        wasEmptyPeriod = false;
        comparator->interruptOn();
        start = end;
    }

    void onComparator() {
        log::debug("onComparator empty=%d high=%d last=%d", wasEmptyPeriod, pin->isHigh(), lastWriteWasHigh);
        if (wasEmptyPeriod) {
            // timeout
            fifo.write((count_t) 0, uint8_t(pin->isHigh() ? 0 : 1));
            comparator->interruptOff();
        } else {
            wasEmptyPeriod = true;
            if (pin->isHigh() != lastWriteWasHigh) {
                onPinChanged();
            } else {
                pin->interruptOnChange();
            }
        }
    }

    PulseCounter(pin_t &_pin, comparator_t &_comparator, count_t _minimumLength):
        comparator(&_comparator), pin(&_pin), minimumLength(_minimumLength) {
        start = comparator->getValue();

        pin->configureAsInputWithPullup();
        pin->interruptOff();

        lastWriteWasHigh = pin->isHigh();
        comparator->setTarget(start + minimumLength);
        comparator->interruptOn();
    }

public:
    template <typename minimumLength_t>
    PulseCounter(comparator_t &_comparator, pin_t &_pin, const minimumLength_t minimumLength):
      PulseCounter(_pin, _comparator, toCountsOn<comparator_t>(minimumLength)) {}

    ~PulseCounter() {
        comparator->interruptOff();
        pin->interruptOff();
    }

    inline uint8_t getOverflows() {
        return fifo.getAbortedWrites();
    }

    void clear() {
        fifo.clear();
    }

    template <typename Body>
    inline void on(Body body) {
        count_t length ;
        uint8_t value;
        if (fifo.read(&length, &value)) {
            body(PulseOn<_comparator_t>(value == 1, length));
        }
    }

    template <typename Body>
    inline void onMax(uint8_t maxPulses, Body body) {
        for (uint8_t i = maxPulses; i > 0; i--) {
            count_t length ;
            uint8_t value;
            if (fifo.read(&length, &value)) {
                body(PulseOn<_comparator_t>(value == 1, length));
            } else {
                return;
            }
        }
    }

    INTERRUPT_HANDLER1(typename comparator_t::INT, onComparator);
    INTERRUPT_HANDLER2(typename pin_t::INT, onPinChanged);
};

template <int fifo_length = 128, typename _comparator_t, typename pin_t, typename minimumLength_t>
inline PulseCounter<_comparator_t,pin_t,fifo_length> pulseCounter(_comparator_t &comparator, pin_t &pin, const minimumLength_t minimumLength) {
    return PulseCounter<_comparator_t,pin_t,fifo_length>(comparator, pin, minimumLength);
}

}

#endif /* PULSECOUNTER_HPP_ */
