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

extern InterruptChain tm0int;
extern InterruptChain tm0ocra;
extern InterruptChain tm0ocrb;
extern InterruptChain tm1int;
extern InterruptChain tm1ocra;
extern InterruptChain tm1ocrb;
extern InterruptChain tm2int;
extern InterruptChain tm2ocra;
extern InterruptChain tm2ocrb;

template <typename info>
class TimerComparator {
public:
    inline void setTarget(typename info::value_t value) {
        *info::ocr = value;
    }
    inline InterruptChain &interrupt() const {
        return *info::handler;
    }
    inline void interruptOn() const {
        *info::tifr |= _BV(info::tifr_bit); // Datasheet: "OCF is cleared by writing logic 1 to the bit"
        *info::timsk |= _BV(info::timsk_bit);
    }
    inline void interruptOff() const {
        *info::timsk &= ~_BV(info::timsk_bit);
    }
};

template <typename info>
class Timer {
public:
    typedef typename info::value_t value_t;
protected:
    void configureFastPWM(const typename info::prescaler_t prescaler) const {
        AtomicScope::SEI _;
        info::configureFastPWM(prescaler);
    }
public:
    inline InterruptChain &interruptOnOverflow() const {
        return *info::intHandler;
    }
    inline void interruptOnOverflowOn() const {
        *info::tifr |= _BV(TOV0); // Datasheet: "[...] is cleared by writing logic 1 to the bit"
        *info::timsk |= _BV(TOIE0);
    }
    inline void interruptOnOverflowOff() const {
        *info::timsk &= ~_BV(TOIE0);
    }
    inline value_t getValue() const {
        return *info::tcnt;
    }
    inline bool isOverflow() const {
        return *info::tifr & _BV(TOV0);
    }
    inline TimerComparator<typename info::ComparatorA> comparatorA() const {
        return TimerComparator<typename info::ComparatorA>();
    }
    inline TimerComparator<typename info::ComparatorB> comparatorB() const {
        return TimerComparator<typename info::ComparatorB>();
    }

    /** 8 for 8-bit timer, 16 for 16-bit timer */
    static constexpr value_t maximum = info::maximum;
    static constexpr uint8_t maximumPower2 = info::maximumPower2;
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

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_1> {
    constexpr static uint8_t power2 = 0;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_8> {
    constexpr static uint8_t power2 = 3;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_32> {
    constexpr static uint8_t power2 = 5;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_64> {
    constexpr static uint8_t power2 = 6;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_128> {
    constexpr static uint8_t power2 = 7;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_256> {
    constexpr static uint8_t power2 = 8;
};

template<> struct PrescalerMeta<IntPrescaler,IntPrescaler::_1024> {
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

    static constexpr InterruptChain* intHandler = &tm0int;

    static constexpr uint8_t maximum = 255;
    static constexpr uint8_t maximumPower2 = 8;

    typedef uint8_t value_t;
    typedef ExtPrescaler prescaler_t;

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept when configuring fast PWM.
        *tccra |= _BV(WGM00);
        *tccra |= _BV(WGM01); // Mode 3, count up to 0xFF
        *tccrb = static_cast<uint8_t>(p);
    }

    struct Comparator {
        typedef uint8_t value_t;
        static constexpr volatile uint8_t *timsk = &TIMSK0;
        static constexpr volatile uint8_t *tifr = &TIFR0;
    };

    struct ComparatorA: public Comparator {
        static constexpr volatile uint8_t *ocr = &OCR0A;
        static constexpr uint8_t timsk_bit = OCIE0A;
        static constexpr uint8_t tifr_bit = OCF0A;
        static constexpr InterruptChain* handler = &tm0ocra;
    };
    struct ComparatorB: public Comparator {
        static constexpr volatile uint8_t *ocr = &OCR0B;
        static constexpr uint8_t timsk_bit = OCIE0B;
        static constexpr uint8_t tifr_bit = OCF0B;
        static constexpr InterruptChain* handler = &tm0ocrb;
    };
};

struct Timer1Info {
    static constexpr volatile uint8_t *tccra = &TCCR1A;
    static constexpr volatile uint8_t *tccrb = &TCCR1B;
    static constexpr volatile uint16_t *tcnt = &TCNT1;
    static constexpr volatile uint8_t *timsk = &TIMSK1;
    static constexpr volatile uint8_t *tifr = &TIFR1;

    static constexpr InterruptChain* intHandler = &tm1int;

    static constexpr uint16_t maximum = 65535;
    static constexpr uint8_t maximumPower2 = 16;

    typedef uint16_t value_t;
    typedef ExtPrescaler prescaler_t;

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept when configuring fast PWM.
        *tccra &= ~_BV(WGM10);
        *tccra |= _BV(WGM11);
        *tccrb = _BV(WGM13) | _BV(WGM12) | static_cast<uint8_t>(p);
        ICR1 = 0xFFFF; // Mode 14, count up to ICR1, which we always set at 0xFFFF.
    }

    struct Comparator {
        typedef uint16_t value_t;
        static constexpr volatile uint8_t *timsk = &TIMSK1;
        static constexpr volatile uint8_t *tifr = &TIFR1;
    };

    struct ComparatorA: public Comparator {
        static constexpr volatile uint16_t *ocr = &OCR1A;
        static constexpr uint8_t timsk_bit = OCIE1A;
        static constexpr uint8_t tifr_bit = OCF1A;
        static constexpr InterruptChain* handler = &tm1ocra;
    };
    struct ComparatorB: public Comparator {
        static constexpr volatile uint16_t *ocr = &OCR1B;
        static constexpr uint8_t timsk_bit = OCIE1B;
        static constexpr uint8_t tifr_bit = OCF1B;
        static constexpr InterruptChain* handler = &tm1ocrb;
    };
};

struct Timer2Info {
    static constexpr volatile uint8_t *tccra = &TCCR2A;
    static constexpr volatile uint8_t *tccrb = &TCCR2B;
    static constexpr volatile uint8_t *tcnt = &TCNT2;
    static constexpr volatile uint8_t *timsk = &TIMSK2;
    static constexpr volatile uint8_t *tifr = &TIFR2;

    static constexpr InterruptChain* intHandler = &tm2int;

    static constexpr uint8_t maximumPower2 = 8;

    typedef uint8_t value_t;
    typedef IntPrescaler prescaler_t;

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept when configuring fast PWM.
        *tccra |= _BV(WGM20);
        *tccra |= _BV(WGM21); // Mode 3, count up to 0xFF
        *tccrb = static_cast<uint8_t>(p);
    }

    struct Comparator {
        typedef uint8_t value_t;
        static constexpr volatile uint8_t *timsk = &TIMSK2;
        static constexpr volatile uint8_t *tifr = &TIFR2;
    };

    struct ComparatorA: public Comparator {
        static constexpr volatile uint8_t *ocr = &OCR2A;
        static constexpr uint8_t timsk_bit = OCIE2A;
        static constexpr uint8_t tifr_bit = OCF2A;
        static constexpr InterruptChain* handler = &tm2ocra;
    };
    struct ComparatorB: public Comparator {
        static constexpr volatile uint8_t *ocr = &OCR2B;
        static constexpr uint8_t timsk_bit = OCIE2B;
        static constexpr uint8_t tifr_bit = OCF2B;
        static constexpr InterruptChain* handler = &tm2ocrb;
    };
};

template <ExtPrescaler prescaler> using Timer0 = PrescaledTimer<Timer0Info,prescaler>;
template <ExtPrescaler prescaler> using Timer1 = PrescaledTimer<Timer1Info,prescaler>;
template <IntPrescaler prescaler> using Timer2 = PrescaledTimer<Timer2Info,prescaler>;

#endif /* TIMERS_HPP_ */
