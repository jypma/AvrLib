/*
 * Timers.hpp
 *
 *  Created on: Dec 25, 2014
 *      Author: jan
 */

#ifndef TIMERS_HPP_
#define TIMERS_HPP_

#include "Interrupt.hpp"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

struct TimerInfo {
    uint16_t const _regA;
    uint16_t const _regB;
    uint16_t const _counter;
    uint16_t const _intFlag;
    uint16_t const _intMask;

    inline volatile uint8_t *regA() const {
        return (volatile uint8_t *)pgm_read_ptr(&_regA);
    }

    inline volatile uint8_t *regB() const {
        return (volatile uint8_t *)pgm_read_ptr(&_regB);
    }

    inline volatile void * counter() const {
        return (volatile void *)pgm_read_ptr(&_counter);
    }

    inline volatile uint8_t * interruptFlag() const {
        return (volatile uint8_t *)pgm_read_ptr(&_intFlag);
    }

    inline volatile uint8_t * interruptMask() const {
        return (volatile uint8_t *)pgm_read_ptr(&_intMask);
    }
};

extern const TimerInfo PROGMEM timerInfos[];

//SIGNAL(TIMER0_OVF_vect);
SIGNAL(TIMER1_OVF_vect);
SIGNAL(TIMER2_OVF_vect);

class TimerInterruptHandler: public InterruptHandler {
    //friend void TIMER0_OVF_vect();
    friend void TIMER1_OVF_vect();
    friend void TIMER2_OVF_vect();
};

template <typename T> class Timer {
private:
    const TimerInfo * const info;
    TimerInterruptHandler _overflow;

    inline volatile uint8_t * const regA() const { return info->regA(); }
    inline volatile uint8_t * const regB() const { return info->regB(); }

    inline void configureFastPWM(uint8_t on, uint8_t off) const {
        *regA() |= (1 << WGM00) | (1 << WGM01);
        *regB() |= on;
        *regB() &= ~((1 << WGM02) | off);
    }

public:
    constexpr Timer(const TimerInfo * const info_): info(info_) {}

    TimerInterruptHandler &interruptOnOverflow() { return _overflow; }

    void interruptOnOverflowOn() {
        *info->interruptMask() |= (1 << TOIE0);
    }

    void configureFastPWM62500Hz() const {
        configureFastPWM((1 << CS00), (1 << CS01) | (1 << CS02));
    }
    /** Actually 7812.5Hz */
    void configureFastPWM7813Hz() const {
        configureFastPWM((1 << CS01), (1 << CS00) | (1 << CS02));
    }
    /** Actually 976.5625 Hz */
    void configureFastPWM977Hz() const {
        configureFastPWM((1 << CS00) | (1 << CS01), (1 << CS02));
    }
    /** Actually 244.140625 Hz */
    void configureFastPWM244Hz() const {
        configureFastPWM((1 << CS02), (1 << CS00) | (1 << CS01));
    }
    /** Actually 61.03515625 Hz */
    void configureFastPWM61Hz() const {
        configureFastPWM((1 << CS02) | (1 << CS00), (1 << CS01));
    }

    /** Returns the timer prescaler: 1, 8, 64, 256 or 1024 */
    uint16_t getPrescaler() const {
        switch(*regB() & ((1 << CS00) | (1 << CS01) | (1 << CS02))) {
            case (1 << CS00): return 1;
            case (1 << CS01): return 8;
            case (1 << CS01) | (1 << CS00): return 64;
            case (1 << CS02): return 256;
            case (1 << CS02) | (1 << CS00): return 1024;
            default: return 1024;
        }
    }

    T getValue() const {
        return *((volatile T *)info->counter());
    }

    bool isOverflow() const {
        return *info->interruptFlag() & (1 << TOV0);
    }
};

extern Timer<uint8_t> timer0;
extern Timer<uint16_t> timer1;
extern Timer<uint8_t> timer2;

class RealTimer {
    Timer<uint8_t> *timer ;
    volatile uint64_t _millis = 0;
    volatile uint16_t _fract = 0;
    volatile uint64_t _ticks = 0;
    uint8_t millisInc;
    uint16_t fractInc;

    uint16_t microsPerCount() const {
        return timer->getPrescaler() / clockCyclesPerMicrosecond;
    }

    void tick() {
        uint64_t m = _millis;
        uint16_t f = _fract;

        m += millisInc;
        f += fractInc;
        if (f >= 1000) {
            f -= 1000;
            m += 1;
        }

        _fract = f;
        _millis = m;
        _ticks++;
    }

    static volatile void doTick(volatile void *ctx) {
        ((RealTimer*)(ctx))->tick();
    }

    static const uint64_t clockCyclesPerMicrosecond = F_CPU / 1000000L;
public:
    RealTimer(Timer<uint8_t> &_timer): timer(&_timer) {
        uint16_t microsPerTick = microsPerCount() * 256L;
        millisInc = microsPerTick / 1000;
        fractInc = microsPerTick % 1000;

        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
        timer->interruptOnOverflowOn();
    }

    uint64_t millis() const {
        ScopedNoInterrupts cli;

        return _millis;
    }

    uint64_t micros() const {
        ScopedNoInterrupts cli;

        uint64_t m = _ticks;
        uint8_t t = timer->getValue();

        if (timer->isOverflow() && (t < 255)) {
            m++;
        }

        return ((m << 8) + t) * microsPerCount();
    }

    void delayMillis(uint64_t ms)
    {
        uint16_t start = (uint16_t)micros();

        while (ms > 0) {
            if (((uint16_t)micros() - start) >= 1000) {
                ms--;
                start += 1000;
            }
        }
    }
};

#endif /* TIMERS_HPP_ */
