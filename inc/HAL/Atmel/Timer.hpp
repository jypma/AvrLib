#ifndef HAL_ATMEL_TIMER_HPP_
#define HAL_ATMEL_TIMER_HPP_

#include <avr/io.h>
#include "Time/Counting.hpp"
#include "Time/Prescaled.hpp"

namespace HAL {
namespace Atmel {
namespace Timer {

template <typename info>
class TimerComparator: public Time::Counting<typename info::value_t> {
public:
    typedef typename info::INT INT;
    typedef typename info::timer_info_t timer_info_t;
    typedef info comparator_info_t;

    static typename info::value_t getValue() {
        return *info::tcnt;
    }
    static void interruptOn() {
        *info::tifr |= _BV(info::tifr_bit); // Datasheet: "OCF is cleared by writing logic 1 to the bit"
        *info::timsk |= _BV(info::timsk_bit);
    }
    static void interruptOff() {
        *info::timsk &= ~_BV(info::timsk_bit);
    }

    static bool isOutputConnected() {
        return (*info::tccra & info::output_mode_bitmask) >> info::output_mode_bitstart != 0;
    }
};

enum class NonPWMOutputMode: uint8_t {
    disconnected = 0,
    toggle_on_match = 1,
    low_on_match = 2,
    high_on_match = 3
};

template <typename info, typename prescaler_t, prescaler_t prescaler>
class NonPWMTimerComparator: public TimerComparator<info>, public Time::Prescaled<typename info::value_t, prescaler_t, prescaler> {
public:
    /**
     * Sets the pin output mode, i.e. what should happen to this comparator's linked
     * pin whenever the comparator matches.
     */
    static void setOutput(NonPWMOutputMode mode) {
        *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(mode) << info::output_mode_bitstart);
    }
    /**
     * Sets the target at which the next comparator match event is to take place.
     * Takes effect immediately.
     */
    static void setTarget(typename info::value_t value) {
        *info::ocr = value;
    }

    /**
     * Forces the value currently given to setOutput() to apply immediately. Calling this method will not generate an interrupt,
     * nor will it affect timer operation.
     */
    static void applyOutput() {
        *info::tccrb |= (1 << info::foc);
    }
};

enum class FastPWMOutputMode: uint8_t {
    disconnected = 0,
    connected = 2,
    connected_inverting = 3
};

template <typename info, typename prescaler_t, prescaler_t prescaler>
class FastPWMTimerComparator: public TimerComparator<info>, public Time::Prescaled<typename info::value_t, prescaler_t, prescaler> {
public:
    /**
     * Sets the pin output mode, i.e. what should happen to this comparator's linked
     * pin whenever the comparator matches.
     */
    static void setOutput(FastPWMOutputMode mode) {
        *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(mode) << info::output_mode_bitstart);
    }
    /**
     * Sets the target at which the next comparator match event is to take place.
     * Takes effect at the start of the next timer run (i.e. after the next overflow).
     */
    static void setTargetFromNextRun(typename info::value_t value) {
        *info::ocr = value;
    }
};

template <typename info, typename comparator_a_t, typename comparator_b_t>
class Timer: public Time::Counting<typename info::value_t> {
public:
    typedef comparator_a_t comparatorA_t;
    typedef comparator_b_t comparatorB_t;
private:
    comparator_a_t comparator_a;
    comparator_b_t comparator_b;
public:
    typedef info timer_info_t;
    typedef typename info::INT INT;

    comparator_a_t &comparatorA() {
        return comparator_a;
    }
    comparator_b_t &comparatorB() {
        return comparator_b;
    }
    static void interruptOnOverflowOn() {
        *info::tifr |= _BV(TOV0); // Datasheet: "[...] is cleared by writing logic 1 to the bit"
        *info::timsk |= _BV(TOIE0);
    }
    static void interruptOnOverflowOff() {
        *info::timsk &= ~_BV(TOIE0);
    }
    static typename info::value_t getValue() {
        return *info::tcnt;
    }
    static bool isOverflow() {
        return *info::tifr & _BV(TOV0);
    }
};

template <typename info, typename info::prescaler_t _prescaler, typename comparator_a_t, typename comparator_b_t>
class PrescaledTimer : public Timer<info, comparator_a_t, comparator_b_t>,
                       public Time::Prescaled<typename info::value_t, typename info::prescaler_t, _prescaler>
{};

/**
 * In fast PWM mode, changes to the comparator values apply on the next timer run.
 * However, hardware PWM can apply those comparator values.
 */
template <typename info, typename info::prescaler_t _prescaler>
class FastPWMTimer: public PrescaledTimer<info, _prescaler,
                      FastPWMTimerComparator<typename info::ComparatorA, typename info::prescaler_t, _prescaler>,
                      FastPWMTimerComparator<typename info::ComparatorB, typename info::prescaler_t, _prescaler>> {
public:
    inline FastPWMTimer() {
        info::configureFastPWM(_prescaler);
    }
    typedef void is_fastpwm;
};

/**
 * In normal mode, changes to the comparator values apply immediately.
 * However, no hardware PWM can be performed.
 */
template <typename info, typename info::prescaler_t _prescaler>
class NormalTimer: public PrescaledTimer<info, _prescaler,
                     NonPWMTimerComparator<typename info::ComparatorA, typename info::prescaler_t, _prescaler>,
                     NonPWMTimerComparator<typename info::ComparatorB, typename info::prescaler_t, _prescaler>> {
public:
    inline NormalTimer() {
        info::configureNormal(_prescaler);
    }
    typedef void is_normal;
};

template <typename info, typename info::prescaler_t _prescaler>
struct TimerDeclaration {
    constexpr TimerDeclaration() {}

    template <uint16_t prescaler>
    constexpr auto withPrescaler() {
        return TimerDeclaration<info, info::template prescalerFromInt<prescaler>()>();
    }

    inline NormalTimer<info, _prescaler> inNormalMode() const {
        return NormalTimer<info, _prescaler>();
    }
};

} // namespace Timer
} // namespace Atmel
} // namespace HAL



#endif /* HAL_ATMEL_TIMER_HPP_ */
