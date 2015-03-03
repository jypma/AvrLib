/*
 * Timers.hpp
 *
 *  Created on: Dec 25, 2014
 *      Author: jan
 */

#ifndef TIMERS_HPP_
#define TIMERS_HPP_

#include "InterruptHandler.hpp"
#include "AtomicScope.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

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

extern TimerInterruptHandler tm0int;
extern TimerInterruptHandler tm1int;
extern TimerInterruptHandler tm2int;

template <typename info>
class Timer {
protected:
    void configureFastPWM(const typename info::prescaler_t prescaler) const {
        AtomicScope::SEI _;

        *info::tccra |= (_BV(WGM00) | _BV(WGM01));
        *info::tccrb = (*info::tccrb & ~_BV(WGM02) & ~_BV(CS00) & ~_BV(CS01) & ~_BV(CS02)) | static_cast<uint8_t>(prescaler);
    }
public:
    InterruptHandler &interruptOnOverflow() const {
        return *info::intHandler;
    }
    void interruptOnOverflowOn() const {
        *info::timsk |= _BV(TOIE0);
    }
    void interruptOnOverflowOff() const {
        *info::timsk &= ~_BV(TOIE0);
    }
    typename info::value_t getValue() const {
        return *info::tcnt;
    }
    bool isOverflow() const {
        return *info::tifr & _BV(TOV0);
    }

};

template<typename prescaler_t, prescaler_t prescaler>
struct PrescalerMeta {};

template<> struct PrescalerMeta<ExtPrescaler,ExtPrescaler::_1> {
    constexpr static uint8_t power2 = 0;
};

template<> struct PrescalerMeta<ExtPrescaler,ExtPrescaler::_8> {
    constexpr static uint8_t power2 = 3;
};

template<> struct PrescalerMeta<ExtPrescaler,ExtPrescaler::_64> {
    constexpr static uint8_t power2 = 6;
};

template<> struct PrescalerMeta<ExtPrescaler,ExtPrescaler::_256> {
    constexpr static uint8_t power2 = 8;
};

template<> struct PrescalerMeta<ExtPrescaler,ExtPrescaler::_1024> {
    constexpr static uint8_t power2 = 10;
};

template <typename info, typename info::prescaler_t _prescaler>
class PrescaledTimer : public Timer<info> {
    typedef PrescalerMeta<typename info::prescaler_t,_prescaler> Meta;
public:
    PrescaledTimer() {
        Timer<info>::configureFastPWM(prescaler);
    }

    static constexpr typename info::prescaler_t prescaler = _prescaler;
    static constexpr uint8_t prescalerPower2 = Meta::power2;
};

struct Timer0Info {
    static constexpr volatile uint8_t *tccra = &TCCR0A;
    static constexpr volatile uint8_t *tccrb = &TCCR0B;
    static constexpr volatile uint8_t *tcnt = &TCNT0;
    static constexpr volatile uint8_t *timsk = &TIMSK0;
    static constexpr volatile uint8_t *tifr = &TIFR0;

    static constexpr TimerInterruptHandler* intHandler = &tm0int;

    typedef uint8_t value_t;
    typedef ExtPrescaler prescaler_t;
};

template <ExtPrescaler prescaler> using Timer0 = PrescaledTimer<Timer0Info,prescaler>;

#endif /* TIMERS_HPP_ */
