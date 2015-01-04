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
#include "Arduino.h"

struct TimerInfo {
    uint16_t const _regA;
    uint16_t const _regB;
    uint16_t const _counter;
    uint16_t const _intFlag;
    uint16_t const _intMask;
    uint8_t const _prescalerPowers[8];

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

    inline uint8_t prescalerPower(uint8_t bits) const {
        return pgm_read_byte((_prescalerPowers) + bits);
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

enum class ExtPrescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _64 = _BV(CS00) | _BV(CS01),
    _256 = _BV(CS02),
    _1024 = _BV(CS02) | _BV(CS00)
};

enum class IntPrescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _32 = _BV(CS00) | _BV(CS01),
    _64 = _BV(CS02),
    _128 = _BV(CS02) | _BV(CS00),
    _256 = _BV(CS02) | _BV(CS01),
    _1024 = _BV(CS02) | _BV(CS01) | _BV(CS00)
};

enum class TimerMode: uint8_t {
    fastPWM
};

class AbstractTimer {
private:
    TimerInterruptHandler _overflow;

    inline volatile uint8_t * const regA() const { return info->regA(); }
    inline volatile uint8_t * const regB() const { return info->regB(); }

protected:
    const TimerInfo * const info;
    void configureFastPWM(const uint8_t prescalerBits) const;

public:
    constexpr AbstractTimer(const TimerInfo * const info_): info(info_) {}

    TimerInterruptHandler &interruptOnOverflow() { return _overflow; }

    void interruptOnOverflowOn() {
        *info->interruptMask() |= _BV(TOIE0);
    }

    void interruptOnOverflowOff() {
        *info->interruptMask() &= ~_BV(TOIE0);
    }

    /** Returns the timer prescaler as 2^N, so prescaler 1 would return 0, prescaler 256 would return 8, etc. */
    uint8_t getPrescalerPower2() const {
        uint8_t prescaler = *regB() & (_BV(CS00) | _BV(CS01) | _BV(CS02));
        return info->prescalerPower(prescaler);
    }

    bool isOverflow() const {
        return *info->interruptFlag() & _BV(TOV0);
    }
};

template<typename size_t, typename prescaler_t>
class Timer: public AbstractTimer {
public:
    constexpr Timer(const TimerInfo * const info_): AbstractTimer(info_) {}

    size_t getValue() const {
        return *((volatile size_t *)info->counter());
    }

    void configure(const TimerMode mode, const prescaler_t prescaler) const {
        switch (mode) {
            case TimerMode::fastPWM:
                configureFastPWM(static_cast<uint8_t>(prescaler));
                break;
        }
    }
};
/*
class Timer8: public Timer {
public:
    constexpr Timer8(const TimerInfo * const info_): Timer(info_) {}

    uint8_t getValue() const {
        return *((volatile uint8_t *)info->counter());
    }
};

class Timer16: public Timer {
public:
    constexpr Timer16(const TimerInfo * const info_): Timer(info_) {}

    uint16_t getValue() const {
        return *((volatile uint16_t *)info->counter());
    }
};
*/

extern Timer<uint8_t,ExtPrescaler> timer0;
extern Timer<uint16_t,ExtPrescaler> timer1;
extern Timer<uint8_t,IntPrescaler> timer2;

template<typename prescaler_t>
class RealTimer {
public:
    Timer<uint8_t,prescaler_t> *timer ;
    volatile uint32_t _millis = 0;
    volatile uint32_t _micros = 0;
    volatile uint32_t _ticks = 0;
    volatile uint32_t millisInc;
    volatile uint32_t microsInc;

    uint16_t _16thMicrosPerCount() const {
        return (1 << timer->getPrescalerPower2()) * 16 / clockCyclesPerMicrosecond;
    }

    void tick() {
       _ticks++;
    }

    static volatile void doTick(volatile void *ctx) {
        ((RealTimer*)(ctx))->tick();
    }

    static const uint64_t clockCyclesPerMicrosecond = F_CPU / 1000000L;
public:
    RealTimer(Timer<uint8_t,prescaler_t> &_timer): timer(&_timer) {
        configure();

        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
        timer->interruptOnOverflowOn();
    }

    RealTimer(Timer<uint8_t,prescaler_t> &_timer, TimerMode mode, prescaler_t prescaler): timer(&_timer) {
        timer->configure(mode, prescaler);
        configure();

        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
        timer->interruptOnOverflowOn();
    }

    uint64_t millis() const {
        ScopedNoInterrupts cli;
        //uint16_t microsPerTick = _16thMicrosPerCount() * 16L * 256L;
        // prescaler 1: 16 * 256 micros per tick, so _ticks * 16 * 256 = _micros, ticks * 16 * 256 / 1000 = _millis
        // prescaler 8: 8 * 16 * 256 micros per tick, so _ticks * 16 * 256 = _micros, ticks * 16 * 256 / 1000 = _millis
        // prescaler 1024: 1024 * 256 clock ticks, 1024 * 245

        /*
        switch (timer->getPrescaler()) {
            case 1: return (((uint64_t)_ticks) * 1 / 16 * 256) / 1000;       // correct
            case 8: return (((uint64_t)_ticks) * 8 / 16 * 256) / 1000;
            case 64: return (((uint64_t)_ticks) * 64 / 16 * 256) / 1000;     // 2 times too many millis
            case 256: return (((uint64_t)_ticks) * 256 / 16 * 256) / 1000;   // 4 times too many millis?
            case 1024: return (((uint64_t)_ticks) * 1024 / 16 * 256) / 1000; // 8 times too many millis?
        }
        */

        return (((uint64_t)_ticks) << timer->getPrescalerPower2()) / 16 * 256 / 1000;
    }
/*
    uint64_t micros() const {
        ScopedNoInterrupts cli;

        uint64_t m = _ticks;
        uint8_t t = timer->getValue();

        if (timer->isOverflow() && (t < 255)) {
            m++;
        }

        return ((m << 8) + t) * _16thMicrosPerCount() / 16;
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
*/
    void configure() {
        ScopedNoInterrupts cli;

        uint16_t microsPerTick = _16thMicrosPerCount() * 16L * 256L;
        millisInc = microsPerTick / 1000;
        microsInc = microsPerTick % 1000;
        _ticks = 0;
    }
};

#endif /* TIMERS_HPP_ */
