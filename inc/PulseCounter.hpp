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

// TODO flip the pulse types, right now HIGH is "transition from low to high", while it should be the actual pulse.

/**
 * Counts up/down pulse lengths on a pin by using a timer. The longest reported length is
 * the timer's maximum value - 1.
 */
template <typename timer_t, timer_t *timer,
          typename comparator_t, comparator_t *comparator,
          typename pin_t, pin_t *pin,
          int fifo_length = 32,
          int minimumLength = 15>
class PulseCounter {
    typedef typename timer_t::value_t count_t;
    typedef PulseCounter<timer_t,timer,comparator_t,comparator,pin_t,pin,fifo_length> This;

public:
    typedef timer_t Timer;
    struct PulseEvent {
        PulseType type;
        count_t length;
    public:
        PulseEvent() {
            type = PulseType::TIMEOUT;
            length = 0;
        }
        PulseEvent(count_t start, count_t end) {
            type = pin->isHigh() ? PulseType::LOW : PulseType::HIGH;
            length = (end > start) ? end - start : timer_t::maximum - (start - end);
            if (length == 0) {
                length = 1;
            }
        }
        inline PulseType getType() const {
            return type;
        }
        inline count_t getLength() const {
            return length;
        }
        uint16_t getLengthAs16384thSeconds() const {
            constexpr uint16_t scale_denom = (uint32_t(F_CPU) >> timer_t::prescalerPower2) / 4;
            uint32_t l = uint32_t(length) * 4096 / scale_denom;
            if (l > 0xffff) {
                l = 0xffff;
            }
            return uint16_t(l);
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
private:

    Fifo<fifo_length> fifo;
    count_t start = timer->getValue();
    bool wasEmptyPeriod = true;

    void onPinChanged() {
        const count_t end = timer->getValue();
        const PulseEvent evt(start, end);
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
    PulseCounter() {
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


#endif /* PULSECOUNTER_HPP_ */
