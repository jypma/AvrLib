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

enum class Prescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _64 = _BV(CS00) | _BV(CS01),
    _256 = _BV(CS02),
    _1024 = _BV(CS02) | _BV(CS00)
};

enum class TimerMode: uint8_t {
    fastPWM
};

struct TimerSettings {
    TimerMode mode;
    Prescaler prescaler;
};

const TimerSettings FastPWM62500Hz = { TimerMode::fastPWM, Prescaler::_1 };
const TimerSettings FastPWM61Hz = { TimerMode::fastPWM, Prescaler::_1024 };

class Timer {
private:
    TimerInterruptHandler _overflow;

    inline volatile uint8_t * const regA() const { return info->regA(); }
    inline volatile uint8_t * const regB() const { return info->regB(); }

    Timer& configureFastPWM(uint8_t on, uint8_t off) const;
protected:
    const TimerInfo * const info;

public:
    constexpr Timer(const TimerInfo * const info_): info(info_) {}

    TimerInterruptHandler &interruptOnOverflow() { return _overflow; }

    void interruptOnOverflowOn() {
        *info->interruptMask() |= _BV(TOIE0);
    }

    void interruptOnOverflowOff() {
        *info->interruptMask() &= ~_BV(TOIE0);
    }

    void configure(const TimerSettings &settings) const;

    inline Timer& configureFastPWM62500Hz() const {
        return configureFastPWM(_BV(CS00), _BV(CS01) | _BV(CS02));
    }
    /** Actually 7812.5Hz */
    inline Timer& configureFastPWM7813Hz() const {
        return configureFastPWM(_BV(CS01), _BV(CS00) | _BV(CS02));
    }
    /** Actually 976.5625 Hz */
    inline Timer& configureFastPWM977Hz() const {
        return configureFastPWM(_BV(CS00) | _BV(CS01), _BV(CS02));
    }
    /** Actually 244.140625 Hz */
    inline Timer& configureFastPWM244Hz() const {
        return configureFastPWM(_BV(CS02), _BV(CS00) | _BV(CS01));
    }
    /** Actually 61.03515625 Hz */
    inline Timer& configureFastPWM61Hz() const {
        return configureFastPWM(_BV(CS02) | _BV(CS00), _BV(CS01));
    }

    /** Returns the timer prescaler: 1, 8, 64, 256 or 1024 */
    uint16_t getPrescaler() const;

    bool isOverflow() const {
        return *info->interruptFlag() & _BV(TOV0);
    }
};

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

extern Timer8 timer0;
extern Timer16 timer1;
extern Timer8 timer2;
extern volatile void *lastCtx;
extern int timers;

class RealTimer {
public:
    Timer8 *timer ;
    volatile uint64_t _millis = 0;
    volatile uint16_t _fract = 0;
    volatile uint64_t _ticks = 0;
    volatile uint8_t millisInc;
    volatile uint16_t fractInc;

    uint16_t _16thMicrosPerCount() const {
        return timer->getPrescaler() * 16 / clockCyclesPerMicrosecond;
    }

    void tick();

    static volatile void doTick(volatile void *ctx) {
        lastCtx = ctx;
        ((RealTimer*)(ctx))->tick();
    }

    static const uint64_t clockCyclesPerMicrosecond = F_CPU / 1000000L;
public:
    RealTimer(Timer8 &_timer): timer(&_timer) {
        timers ++;
//        Serial.println("Constructing");
//        Serial.flush();

        //timer->configureFastPWM7813Hz();
        timer->configure(FastPWM62500Hz);
        configure();
 //       Serial.println(timer->getPrescaler());
 //       Serial.flush();

  //      Serial.println((unsigned long) clockCyclesPerMicrosecond);
  //      Serial.flush();

   //     Serial.println(fractInc);
  //      Serial.flush();

        lastCtx = (volatile void *)this;
        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
   //     Serial.println("attached");
   //     Serial.flush();

        timer->interruptOnOverflowOn();
   //     Serial.println("on");
   //     Serial.flush();

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

    void configure() {
        uint16_t microsPerTick = _16thMicrosPerCount() * 16L * 256L;
        millisInc = microsPerTick / 1000;
        fractInc = microsPerTick % 1000;
    }
};

#endif /* TIMERS_HPP_ */
