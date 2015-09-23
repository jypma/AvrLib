/*
 * PulseCounter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jan
 */

#ifndef PULSECOUNTER_HPP_
#define PULSECOUNTER_HPP_

#include "Fifo.hpp"
#include "Timer.hpp"
#include "Streams/Streamable.hpp"
#include "Serial/Pulse.hpp"

namespace Serial {

using namespace Streams;
using namespace Time;



/**
 * TODO idea to rate limit interrupts: only check once every (timer_overflow) us.
 *
onINT(timer_overflow) {
    re-enable onchange interrupt
    if changed in the meantime, register change.
}
 *
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

private:

    Fifo<fifo_length> fifo;
    volatile count_t start;
    volatile bool wasEmptyPeriod = true;
    bool lastWasHigh;

    _comparator_t *const comparator;
    pin_t *const pin;
    const count_t minimumLength;

    void onPinChanged() {
        const count_t end = comparator->getValue();
        const count_t length = (end > start) ? end - start :
                               Counting<count_t>::maximum - (start - end);
        if (length > minimumLength) {
            fifo.out() << length;
        }
        wasEmptyPeriod = false;
        comparator->interruptOn();
        start = end;
    }

    void onComparator() {
        if (wasEmptyPeriod) {
            if (!(fifo.out() << ((count_t) 0))) {
                fifo.clear();
                fifo.out() << ((count_t) 0);
            }
            comparator->interruptOff();
        } else {
            wasEmptyPeriod = true;
        }
    }

    InterruptHandler comp = { this, &This::onComparator };
    InterruptHandler chng = { this, &This::onPinChanged };

public:
    PulseCounter(comparator_t &_comparator, pin_t &_pin, count_t _minimumLength = 15):
        comparator(&_comparator), pin(&_pin), minimumLength(_minimumLength) {
        start = comparator->getValue();

        comparator->interruptOff();
        comparator->interrupt().attach(comp);
        comparator->setTarget(0);

        pin->configureAsInputWithPullup();
        pin->interrupt().attach(chng);
        pin->interruptOnChange();

        lastWasHigh = pin->isHigh();
    }

    ~PulseCounter() {
        comparator->interruptOff();
        comparator->interrupt().detach();
        pin->interruptOff();
        pin->interrupt().detach();
    }

    inline uint8_t getOverflows() {
        return fifo.getAbortedWrites();
    }

    template <typename Body>
    inline void on(Body body) {
        count_t length ;
        if (fifo.in() >> length) {
            body(Pulse(lastWasHigh, length));
            lastWasHigh = !lastWasHigh;
        }
    }

    template <typename Body>
    inline void onMax(uint8_t maxPulses, Body body) {
        for (uint8_t i = maxPulses; i > 0; i--) {
            count_t length ;
            if (fifo.in() >> length) {
                body(Pulse(lastWasHigh, length));
                lastWasHigh = !lastWasHigh;
            } else {
                return;
            }
        }
    }
};

template <int fifo_length = 32, typename _comparator_t, typename pin_t, typename minimumLength_t>
inline PulseCounter<_comparator_t,pin_t,fifo_length> pulseCounter(_comparator_t &comparator, pin_t &pin, const minimumLength_t minimumLength) {
    return PulseCounter<_comparator_t,pin_t,fifo_length>(comparator, pin, toCountsOn(comparator, minimumLength));
}

}

#endif /* PULSECOUNTER_HPP_ */
