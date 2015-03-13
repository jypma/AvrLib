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

enum class PulseType: uint8_t { HIGH, LOW, TIMEOUT };

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
    typedef PulseCounter<timer_t,timer,comparator_t,comparator,pin_t,pin,fifo_length> This;

public:
    struct PulseEvent {
        PulseType type;
        count_t length;
    public:
        PulseEvent() {
            type = PulseType::TIMEOUT;
            length = 0;
        }
        PulseEvent(count_t start) {
            type = pin->isHigh() ? PulseType::HIGH : PulseType::LOW;
            count_t end = timer->getValue();
            length = (end > start) ? end - start : timer_t::maximum - (start - end);
            if (length == 0) {
                length = 1;
            }
        }
        inline PulseType getType() {
            return type;
        }
        inline count_t getLength() {
            return length;
        }
        uint16_t getLengthAs16384thSeconds() {
            constexpr uint16_t scale_denom = (F_CPU >> Timer1<ExtPrescaler::_1024>::prescalerPower2) / 4;
            uint32_t l = length * 4096 / scale_denom;
            if (l > 0xffff) {
                l = 0xffff;
            }
            return uint16_t(l);
        }
        static void write(Writer &out, const PulseEvent &evt) {
            out << evt.type;
            if (evt.type != PulseType::TIMEOUT) {
                out << evt.length;
            }
        }
        static void read(Reader &in, PulseEvent &evt) {
            if (in >> evt.type) {
                if (evt.type != PulseType::TIMEOUT) {
                    in >> evt.length;
                }
            }
        }
    };
private:

    Fifo<fifo_length> fifo;
    count_t start = timer->getValue();
    bool wasEmptyPeriod = true;

    void onPinChanged() {
        PulseEvent evt(start);
        fifo.out() << evt;
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
