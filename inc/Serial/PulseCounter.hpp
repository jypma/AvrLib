/*
 * PulseCounter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jan
 */

#ifndef PULSECOUNTER_HPP_
#define PULSECOUNTER_HPP_

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Fifo.hpp"
#include "HAL/Atmel/Timer.hpp"
#include "Serial/Pulse.hpp"
#include "Logging.hpp"

extern volatile uint16_t pls;
namespace Serial {

using namespace Streams;
using namespace Time;
using namespace HAL::Atmel::InterruptHandlers;


/**
 * Counts up/down pulse lengths on a pin by using a timer.
 */
template <typename _comparator_t,
          typename pin_t,
          int fifo_length = 32>
class PulseCounter {
public:
    typedef PulseCounter<_comparator_t,pin_t,fifo_length> This;
    typedef _comparator_t comparator_t;
    typedef typename comparator_t::value_t count_t;
    PulseCounter (const This &) = default;

private:
    typedef Logging::Log<Loggers::Serial> log;
    static_assert(!log::isDebugEnabled() || comparator_t::prescalerPower2 > 3,
            "You should disable debug logging on Serial if using pulse counter with prescaler <= 8");

    Fifo<fifo_length> fifo;
    volatile count_t start;
    uint8_t space;

    _comparator_t *const comparator;
    pin_t *const pin;

    void onPinChanged() { // this method takes 90 cycles for 8-bit timers
    	log::timeStart();

    	pls++;
        const count_t end = comparator->getValue();
        const count_t length = (end > start) ? end - start :
                               Counting<count_t>::maximum - (start - end);

    	if (space <= 1) {
    		space = fifo.fastGetSpace();
    	}
    	if (space > 1) {
    		//fifo.fastwrite(length, uint8_t(pin->isHigh() ? 0 : 1));
    		fifo.fastUncheckedWrite(length);
    		fifo.fastUncheckedWrite(uint8_t(pin->isHigh() ? 0 : 1));
    	}

        //log::debug(F("P s="), dec(start), F(" e="), dec(end), F(" l="), dec(lastWriteWasHigh));

        comparator->setTarget(end - 1);
        comparator->interruptOn();
        start = end;
        log::timeEnd();
    }

    void onComparator() {
        //log::debug(F("C e="), dec(wasEmptyPeriod), F(" p="), dec(pin->isHigh()), F(" l="), dec(lastWriteWasHigh));
        // timeout
        fifo.fastwrite((count_t) 0, uint8_t(pin->isHigh() ? 0 : 1));
        comparator->interruptOff();
    }

    void go() {
    	space = fifo.getSpace();
        start = comparator->getValue();
        comparator->setTarget(start - 1);
        comparator->interruptOn();
        pin->interruptOnChange();
    }
public:
    typedef On<This, typename _comparator_t::INT, &This::onComparator,
            On<This, typename pin_t::INT, &This::onPinChanged>> Handlers;

    PulseCounter(comparator_t &_comparator, pin_t &_pin):
        comparator(&_comparator), pin(&_pin) {
        pin->configureAsInputWithPullup();
        go();
    }

    void pause() {
        comparator->interruptOff();
        pin->interruptOff();
    }

    ~PulseCounter() {
        pause();
    }

    inline uint8_t getOverflows() {
        return fifo.getAbortedWrites();
    }

    void clear() {
        fifo.clear();
    }

    void resume() {
        AtomicScope _;
        fifo.clear();
        go();
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
};

template <int fifo_length = 128, typename _comparator_t, typename pin_t, typename minimumLength_t>
inline PulseCounter<_comparator_t,pin_t,fifo_length> pulseCounter(_comparator_t &comparator, pin_t &pin) {
    return PulseCounter<_comparator_t,pin_t,fifo_length>(comparator, pin);
}

/**
 * Counts up/down pulse lengths on a pin by using a timer. The longest reported length is
 * the timer's maximum value - 1.
 */
template <typename _comparator_t,
          typename pin_t,
          int fifo_length = 32>
class MinPulseCounter {
public:
    typedef MinPulseCounter<_comparator_t,pin_t,fifo_length> This;
    typedef _comparator_t comparator_t;
    typedef typename comparator_t::value_t count_t;
    MinPulseCounter (const This &) = default;

private:
    typedef Logging::Log<Loggers::Serial> log;

    Fifo<fifo_length> fifo;
    volatile count_t start;
    volatile bool wasEmptyPeriod = false;
    bool lastWriteWasHigh;

    _comparator_t *const comparator;
    pin_t *const pin;
    const count_t minimumLength;

    __attribute__((always_inline)) void onPinChanged() { // this method takes 169 cycles
        const bool hi = pin->isHigh();
        const bool repeat = hi == lastWriteWasHigh;
        lastWriteWasHigh = hi;

        const count_t end = comparator->getValue();
        const count_t length = (end > start) ? end - start :
                               Counting<count_t>::maximum - (start - end);

        //log::debug(F("P s="), dec(start), F(" e="), dec(end), F(" l="), dec(lastWriteWasHigh));
        if (repeat) {
            fifo.fastwrite(1, uint8_t(hi ? 1 : 0));
        }
        fifo.fastwrite(length, uint8_t(hi ? 0 : 1));

        start = end;

        const auto sleepEnd = end + minimumLength;
        if (sleepEnd > comparator->getValue() + 2) {
            // don't count pulses until sleepEnd
            wasEmptyPeriod = false;
            pin->interruptOff();
            comparator->setTarget(sleepEnd);
        } else {
            // we were too slow in out interrupt handler, just return and wait for the next pin change.
            // TODO actually measure how long the above code takes, so we can statically catch that.
            wasEmptyPeriod = true;
            comparator->setTarget(end - 2);
        }
        comparator->interruptOn();
    }

    void onComparator() { // this method takes 62 cycles if waking up unchanged, 185 cycles if waking up missing a pin change.
        //log::debug(F("C e="), dec(wasEmptyPeriod), F(" p="), dec(pin->isHigh()), F(" l="), dec(lastWriteWasHigh));
        if (wasEmptyPeriod) {
            // timeout
            fifo.fastwrite((count_t) 0, uint8_t(pin->isHigh() ? 0 : 1));
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

    MinPulseCounter(pin_t &_pin, comparator_t &_comparator, count_t _minimumLength):
        comparator(&_comparator), pin(&_pin), minimumLength(_minimumLength) {
        start = comparator->getValue();

        pin->configureAsInputWithPullup();
        pin->interruptOff();

        lastWriteWasHigh = pin->isHigh();
        log::debug(F("start at "), dec(start + minimumLength));
        comparator->setTarget(start + minimumLength);
        comparator->interruptOn();
    }

public:
    typedef On<This, typename comparator_t::INT, &This::onComparator,
            On<This, typename pin_t::INT, &This::onPinChanged>> Handlers;

    template <typename minimumLength_t>
    MinPulseCounter(comparator_t &_comparator, pin_t &_pin, const minimumLength_t minimumLength):
        MinPulseCounter(_pin, _comparator, toCountsOn<comparator_t>(minimumLength)) {}

    ~MinPulseCounter() {
        comparator->interruptOff();
        pin->interruptOff();
    }

    inline uint8_t getOverflows() {
        return fifo.getAbortedWrites();
    }

    void clear() {
        AtomicScope _;

        fifo.clear();
        lastWriteWasHigh = pin->isHigh();
        pin->interruptOff();
        start = comparator->getValue();
        wasEmptyPeriod = false;
        comparator->setTarget(start + minimumLength);
        comparator->interruptOn();
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

    void pause() {
        comparator->interruptOff();
        pin->interruptOff();
    }

    void resume() {
        clear();
    }
};

template <int fifo_length = 128, typename _comparator_t, typename pin_t, typename minimumLength_t>
inline MinPulseCounter<_comparator_t,pin_t,fifo_length> pulseCounter(_comparator_t &comparator, pin_t &pin, const minimumLength_t minimumLength) {
    // The extra code required to do minimum length checking takes about 12.5us. Hence, it only makes sense to do that
    // for minimumLength >= 25us.
    static_assert(uint64_t(toMicrosOn<_comparator_t,minimumLength_t>()) > 25,
        "minimumLength should at least be 25us. If expecting pulses smaller than that, use a plain pulsecounter without minimum.");

    return MinPulseCounter<_comparator_t,pin_t,fifo_length>(comparator, pin, minimumLength);
}

}

#endif /* PULSECOUNTER_HPP_ */
