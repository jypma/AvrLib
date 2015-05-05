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
#include "Reader.hpp"
#include "Writer.hpp"

enum class PulseType: uint8_t { TIMEOUT = 0, HIGH = 1, LOW = 2 };

template <typename _value_t>
struct PulseEvent: public Counting<_value_t> {
    PulseType type;
    _value_t length;
public:
    PulseEvent() {
        type = PulseType::TIMEOUT;
        length = 0;
    }

    PulseEvent(PulseType _type, _value_t _length): type(_type), length(_length) {}

    PulseEvent(_value_t start, _value_t end, bool high) {
        type = high ? PulseType::LOW : PulseType::HIGH;
        length = (end > start) ? end - start : Counting<_value_t>::maximum - (start - end);
        if (length == 0) {
            length = 1;
        }
    }
    inline PulseType getType() const {
        return type;
    }
    inline _value_t getLength() const {
        return length;
    }

    static void write(Writer &out, const PulseEvent &evt) {
        if (evt.type == PulseType::TIMEOUT) {
            out << uint8_t(0);
        } else {
            uint8_t marker = uint8_t(evt.type);
            if (evt.length > 255) {
                marker |= 128;
                out << marker << evt.length;
            } else {
                out << marker << uint8_t(evt.length);
            }
        }
    }
    static void read(Reader &in, PulseEvent &evt) {
        uint8_t type;
        if (in >> type) {
            if (type == 0) {
                evt.type = PulseType::TIMEOUT;
            } else {
                evt.type = PulseType(type & 0b11);
                if (type & 128) {
                    in >> evt.length;
                } else {
                    uint8_t length;
                    in >> length;
                    evt.length = length;
                }
            }
        } else {
            evt.type = PulseType::TIMEOUT;
        }
    }
};

/**
 * Counts up/down pulse lengths on a pin by using a timer. The longest reported length is
 * the timer's maximum value - 1.
 */
template <typename _comparator_t,
          typename pin_t,
          int fifo_length = 32>
class PulseCounter {
public:
    typedef PulseCounter<_comparator_t,pin_t> This;
    typedef _comparator_t comparator_t;
    typedef typename comparator_t::value_t count_t;

private:

    Fifo<fifo_length> fifo;
    count_t start = comparator->getValue();
    bool wasEmptyPeriod = true;

    _comparator_t *const comparator;
    pin_t *const pin;
    const count_t minimumLength;

    void onPinChanged() {
        const count_t end = comparator->getValue();
        const PulseEvent<count_t> evt(start, end, pin->isHigh());
        if (evt.getLength() > minimumLength) {
           fifo.out() << evt;
        } /*else {
           TODO consider fifo.clear() here? or what?
           fifo.out() << PulseType::TIMEOUT;
        }*/
        wasEmptyPeriod = false;
        comparator->interruptOn();
        start = end;
    }

    void onComparator() {
        if (wasEmptyPeriod) {
            if (!(fifo.out() << PulseType::TIMEOUT)) {
                fifo.clear();
                fifo.out() << PulseType::TIMEOUT;
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

        comparator->interruptOff();
        comparator->interrupt().attach(comp);
        comparator->setTarget(0);

        pin->interrupt().attach(chng);
        pin->interruptOnChange();
    }

    ~PulseCounter() {
        comparator->interruptOff();
        comparator->interrupt().detach();
        pin->interruptOff();
        pin->interrupt().detach();
    }

    inline Reader in() {
        return fifo.in();
    }
};

template <typename _comparator_t, typename pin_t, int fifo_length = 32>
PulseCounter<_comparator_t,pin_t,fifo_length> pulseCounter(_comparator_t &comparator, pin_t &pin, int minimumLength = 15) {
    return PulseCounter<_comparator_t,pin_t,fifo_length>(comparator, pin, minimumLength);
}

#endif /* PULSECOUNTER_HPP_ */
