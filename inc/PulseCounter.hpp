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

/**
 * Counts up/down pulse lengths on a pin by using a timer. The longest reported length is
 * the timer's maximum value - 1.
 */
template <typename timer_t, timer_t *timer,
          typename comparator_t, comparator_t *comparator,
          typename pin_t, pin_t *pin,
          int fifo_length = 16>
class PulseCounter {
    typedef typename timer_t::value_t count_t;

    Fifo<fifo_length> fifo;
    count_t start = timer->getValue();
    bool wasEmptyPeriod = true;

    void onPinChanged() {
        fifo.append(pin->isHigh() ? 1 : 0);
        count_t end = timer->getValue();
        count_t duration = (end > start) ? end - start : timer_t::maximum - (start - end);
        fifo.out() << duration;
        wasEmptyPeriod = false;
        comparator->interruptOn();
    }
    void onComparator() {
        if (wasEmptyPeriod) {
            fifo.append(2);
            comparator->interruptOff();
        } else {
            wasEmptyPeriod = true;
        }
    }

    static void doOnComparator(volatile void *ctx) {
        ((PulseCounter<timer_t,timer,comparator_t,comparator,pin_t,pin,fifo_length>*)ctx)->onComparator();
    }
    static void doOnChange(volatile void *ctx) {
        ((PulseCounter<timer_t,timer,comparator_t,comparator,pin_t,pin,fifo_length>*)ctx)->onPinChanged();
    }


public:
    PulseCounter() {
        comparator->interruptOff();
        comparator->interrupt().attach(&doOnComparator, this);
        comparator->setTarget(0);

        pin->interrupt().attach(&doOnChange, this);
        pin->interruptOnChange();
    }
    ~PulseCounter() {
        //TODO detach
        comparator->interruptOff();
        pin->interruptOff();
    }

    inline Reader in() {
        return fifo.in();
    }
};


#endif /* PULSECOUNTER_HPP_ */
