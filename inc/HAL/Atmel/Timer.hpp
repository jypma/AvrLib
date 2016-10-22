#ifndef HAL_ATMEL_TIMER_HPP_
#define HAL_ATMEL_TIMER_HPP_

#include "Time/Counting.hpp"
#include "Time/Prescaled.hpp"
#include "Time/TimerValue.hpp"

namespace HAL {
namespace Atmel {

enum class NonPWMOutputMode: uint8_t {
    disconnected = 0,
    toggle_on_match = 1,
    low_on_match = 2,
    high_on_match = 3
};

enum class FastPWMOutputMode: uint8_t {
    disconnected = 0,
    connected = 2,
    connected_inverting = 3
};

namespace Timer {

template <typename info>
class TimerComparator: public Time::Counting<typename info::timer_info_t::value_t> {
public:
    typedef typename info::INT INT;
    typedef typename info::timer_info_t timer_info_t;
    typedef info comparator_info_t;

    static typename timer_info_t::value_t getValue() {
      return timer_info_t::TCNT.val();
    }
    __attribute__((always_inline)) inline static void interruptOn() {
    	info::OCF.set();  // Datasheet: "OCF is cleared by writing logic 1 to the bit"
        info::OCIE.set();
    }
    __attribute__((always_inline)) inline static void interruptOff() {
        info::OCIE.clear();
    }

    static bool isOutputConnected() {
    	return info::COM0.isSet() || info::COM1.isSet();
    }
};

template <typename info, typename prescaler_t, prescaler_t prescaler>
class NonPWMTimerComparator: public TimerComparator<info>,
                             public Time::Prescaled<typename info::timer_info_t::value_t, prescaler_t, prescaler> {
public:
	typedef NonPWMTimerComparator<info, prescaler_t, prescaler> This;
    typedef ::Time::TimerValue<This> timervalue_t;

    /**
     * Sets the pin output mode, i.e. what should happen to this comparator's linked
     * pin whenever the comparator matches.
     */
    static void setOutput(NonPWMOutputMode mode) {
    	info::COM0.apply(static_cast<uint8_t>(mode) & 1);
    	info::COM1.apply(static_cast<uint8_t>(mode) & 2);
    }

    static NonPWMOutputMode getOutput() {
    	return static_cast<NonPWMOutputMode> ((1 & info::COM0.getValue()) | (2 & info::COM1.getValue()));
    }

    /**
     * Sets the target at which the next comparator match event is to take place.
     * Takes effect immediately.
     */
    static void setTarget(timervalue_t value) {
      info::OCR.val() = value;
    }

    static timervalue_t getTarget() {
      return info::OCR.val();
    }

    /**
     * Forces the value currently given to setOutput() to apply immediately. Calling this method will not generate an interrupt,
     * nor will it affect timer operation.
     */
    static void applyOutput() {
        info::FOC.set();
    }
};

template <typename info, typename prescaler_t, prescaler_t prescaler>
class FastPWMTimerComparator: public TimerComparator<info>, public Time::Prescaled<typename info::value_t, prescaler_t, prescaler> {
public:
	typedef FastPWMTimerComparator<info, prescaler_t, prescaler> This;
    typedef ::Time::TimerValue<This> timervalue_t;

    /**
     * Sets the pin output mode, i.e. what should happen to this comparator's linked
     * pin whenever the comparator matches.
     */
    static void setOutput(FastPWMOutputMode mode) {
    	info::COM0.apply(static_cast<uint8_t>(mode) & 1);
    	info::COM1.apply(static_cast<uint8_t>(mode) & 2);
    }

    static FastPWMOutputMode getOutput() {
    	return static_cast<FastPWMOutputMode> ((1 & info::COM0.getValue()) | (2 & info::COM1.getValue()));
    }

    /**
     * Sets the target at which the next comparator match event is to take place.
     * Takes effect at the start of the next timer run (i.e. after the next overflow).
     */
    static void setTargetFromNextRun(timervalue_t value) {
        info::OCR.set(value);
    }

    static typename info::value_t getTarget() {
        return info::OCR;
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
    	info::TOV.set();    // Datasheet: "[...] is cleared by writing logic 1 to the bit"
    	info::TOIE.set();
    }
    static void interruptOnOverflowOff() {
    	info::TOIE.clear();
    }
    static typename info::value_t getValue() {
        return info::TCNT;
    }
    static bool isOverflow() {
    	return info::TOV.isSet();
    }
};

template <typename info, typename info::prescaler_t _prescaler, typename comparator_a_t, typename comparator_b_t>
class PrescaledTimer : public Timer<info, comparator_a_t, comparator_b_t>,
                       public Time::Prescaled<typename info::value_t, typename info::prescaler_t, _prescaler>
{
public:
	typedef PrescaledTimer<info,_prescaler,comparator_a_t,comparator_b_t> This;
    typedef ::Time::TimerValue<This> timervalue_t;

    static timervalue_t getValue() {
      return info::TCNT.val();
    }
};

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

    template <uint16_t i> using withPrescaler = TimerDeclaration<info, info::template PrescalerFromInt<i>::value>;

    constexpr static NormalTimer<info, _prescaler> inNormalMode() {
        return NormalTimer<info, _prescaler>();
    }

    constexpr static FastPWMTimer<info, _prescaler> inFastPWMMode() {
        return FastPWMTimer<info, _prescaler>();
    }
};

} // namespace Timer
} // namespace Atmel
} // namespace HAL



#endif /* HAL_ATMEL_TIMER_HPP_ */
