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
    volatile uint8_t * const _regA;
    volatile uint8_t * const _regB;
    volatile void * const _counter;
    volatile uint8_t * const _intFlag;
    volatile uint8_t * const _intMask;
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
        return pgm_read_byte(_prescalerPowers + bits);
    }
};

extern const TimerInfo PROGMEM timerInfos[];

ISR(TIMER0_OVF_vect);
ISR(TIMER1_OVF_vect);
ISR(TIMER2_OVF_vect);

class TimerInterruptHandler: public InterruptHandler {
    friend void TIMER0_OVF_vect();
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

#if F_CPU == 16000000
constexpr auto IntPrescaler62500Hz = IntPrescaler::_1;
constexpr auto IntPrescaler7813Hz = IntPrescaler::_8;
constexpr auto IntPrescaler1953Hz = IntPrescaler::_32;
constexpr auto IntPrescaler977Hz = IntPrescaler::_64;
constexpr auto IntPrescaler488Hz = IntPrescaler::_128;
constexpr auto IntPrescaler244Hz = IntPrescaler::_256;
constexpr auto IntPrescaler61Hz = IntPrescaler::_1024;

constexpr auto ExtPrescaler62500Hz = ExtPrescaler::_1;
constexpr auto ExtPrescaler7813Hz = ExtPrescaler::_8;
constexpr auto ExtPrescaler977Hz = ExtPrescaler::_64;
constexpr auto ExtPrescaler244Hz = ExtPrescaler::_256;
constexpr auto ExtPrescaler61Hz = ExtPrescaler::_1024;
#endif

enum class TimerMode: uint8_t {
    fastPWM
};

class AbstractTimer {
private:
    TimerInterruptHandler _overflow;

    inline volatile uint8_t * regA() const { return info->regA(); }
    inline volatile uint8_t * regB() const { return info->regB(); }

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

extern Timer<uint8_t,ExtPrescaler> timer0;
extern Timer<uint16_t,ExtPrescaler> timer1;
extern Timer<uint8_t,IntPrescaler> timer2;

#endif /* TIMERS_HPP_ */
