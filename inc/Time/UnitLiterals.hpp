#pragma once

#include "RuntimeUnits.hpp"
#include "gcc_limits.h"
#include "gcc_type_traits.h"

namespace Time {

namespace Lit {

template <uint64_t value> class Counts;
template <uint64_t value> class Ticks;
template <uint64_t value> class Milliseconds;
template <uint64_t value> class Microseconds;

template <typename T, T _value>
class Truncatable {
    template <typename I>
    static constexpr bool isValid() {
        return _value <= std::numeric_limits<I>::max();
    }

public:
    static constexpr T value = _value;

    template <typename I>
    constexpr operator I() {
        static_assert(isValid<I>(),
                "A value does not fit in return_t, you might want to widen the return type, or in case of a timed value, increase the timer prescaler.");
        return _value;
    }

    template <typename int_t>
    constexpr bool operator >(const int_t b) { return _value > uint64_t(b); }

    static constexpr bool is_uint8 = isValid<uint8_t>();
    static constexpr bool is_uint16 = isValid<uint16_t>();
    static constexpr bool is_uint32 = isValid<uint32_t>();
};

template <uint64_t value> using ut64_t = Truncatable<uint64_t, value>;

template<uint64_t value, typename R, template<uint64_t> class This>
struct LiteralTimeUnit {
    static constexpr This<value> instance = {};

	static constexpr ut64_t<value> getValue() {
		return {};
	}

	static constexpr uint64_t getAmount() {
	    return value;
	}

    template <uint64_t percentage>
    static constexpr This<value * percentage / 100> percent() { return {}; }

    template <uint64_t factor>
    static constexpr This<value * factor> times() { return {}; }

    template <uint64_t divisor>
    static constexpr This<value / divisor> dividedBy() { return {}; }

    constexpr bool operator== (const R that) const { return that.getValue() == value; }

    template <uint64_t value2>
    constexpr bool operator> (const This<value2> that) const { return value > value2; }

    template <typename int_t>
    constexpr R operator* (const int_t factor) const { return (operator R()) * factor; }

    constexpr operator R () const {
    	return R((uint32_t) ut64_t<value>());
    }
};

template <uint64_t percentage>
struct PercentApply {
	template <typename value_t>
	constexpr auto operator()(value_t v) -> decltype(value_t::template percent<percentage>()) {
		return {};
	}
};

template<uint64_t factor, typename prescaled_t>
struct UnitsPerMillis {
	static constexpr double value = double(uint64_t(F_CPU) / 1000) / (1 << prescaled_t::prescalerPower2) / factor;
	static constexpr double max = 0xFFFFFFFF * value;
};

template<uint64_t v, uint64_t factor, typename prescaled_t, typename check = void>
struct UnitsToMillis {
	static constexpr Milliseconds<uint64_t(v * UnitsPerMillis<factor,prescaled_t>::value)> value = {};
};

template<uint64_t v, uint64_t factor, typename prescaled_t>
struct UnitsToMillis<v,factor,prescaled_t,typename std::enable_if<v >= UnitsPerMillis<factor,prescaled_t>::max>::type> {
	//static constexpr Milliseconds<0xFFFFFFFF> value = {};
};

template<uint64_t factor, typename prescaled_t>
struct UnitsPerMicros {
	static constexpr double value = double(uint64_t(F_CPU) / 1000000) / (1 << prescaled_t::prescalerPower2) / factor;
	static constexpr double max = 0xFFFFFFFF * value;
};

template<uint64_t v, uint64_t factor, typename prescaled_t, typename check = void>
struct UnitsToMicros {
	static constexpr Microseconds<uint64_t(v * UnitsPerMillis<factor,prescaled_t>::value)> value = {};
};

template<uint64_t v, uint64_t factor, typename prescaled_t>
struct UnitsToMicros<v,factor,prescaled_t,typename std::enable_if<v >= UnitsPerMicros<factor,prescaled_t>::max>::type> {
	//static constexpr Microseconds<0xFFFFFFFF> value = {};
};

/**
 * Counts represent individual timer increments.
 */
template <uint64_t value>
struct Counts: public LiteralTimeUnit<value, ::Time::Counts, Counts> {
	typedef LiteralTimeUnit<value, ::Time::Counts, Counts> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    static constexpr Counts<value> toCounts() { return {}; }

    template <typename prescaled_t>
    static constexpr Counts<value> toCountsOn() { return {}; }

    template <typename prescaled_t>
    static constexpr auto toTicksOn() {
        constexpr Ticks<value / (prescaled_t::maximum + 1)> result = {} ;
        static_assert(result.getValue() > 1,
                "Number of ticks for counts is so low that it rounds to 0 or 1, you shouldn't use this counts value as ticks.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr auto toMillisOn() { return UnitsToMillis<value,1,prescaled_t>::value; }

    template <typename prescaled_t>
    static constexpr auto toMicrosOn() { return UnitsToMicros<value,1,prescaled_t>::value; }
};

/**
 * Ticks represent timer overflows, i.e. every 256th or 65kth count, depending on timer size.
 */
template <uint64_t value>
struct Ticks: public LiteralTimeUnit<value, ::Time::Ticks, Ticks> {
	typedef LiteralTimeUnit<value, ::Time::Ticks, Ticks> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    template <typename prescaled_t>
    static constexpr Counts<value * (prescaled_t::maximum + 1)> toCountsOn() { return {}; }

    template <typename prescaled_t>
    static constexpr Ticks<value> toTicks() { return {}; }

    template <typename prescaled_t>
    static constexpr Ticks<value> toTicksOn() { return {}; }

    template <typename prescaled_t>
    static constexpr auto toMillisOn() { return UnitsToMillis<value,(prescaled_t::maximum + 1),prescaled_t>::value; }

    template <typename prescaled_t>
    static constexpr auto toMicrosOn() { return UnitsToMicros<value,(prescaled_t::maximum + 1),prescaled_t>::value; }
};

template <uint64_t value>
struct Microseconds: public LiteralTimeUnit<value, ::Time::Microseconds, Microseconds> {
	typedef LiteralTimeUnit<value, ::Time::Microseconds, Microseconds> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    template <typename prescaled_t>
    static constexpr auto toCountsOn() {
        constexpr Counts<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) / 1000 * value / 1000> result = {};
        static_assert(result.getValue() > 1,
                "Number of counts for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr auto toTicksOn() {
        constexpr Ticks<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) / 1000 / (uint64_t(prescaled_t::maximum) + 1) * value / 1000> result = {};
        static_assert(result.getValue() > 1,
                "Number of ticks for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr Microseconds<value> toMicrosOn() { return {}; }

    static constexpr Microseconds<value> toMicros() { return {}; }

    template <typename prescaled_t>
    static constexpr Milliseconds<value / 1000> toMillisOn() { return {}; }

    static constexpr Milliseconds<value / 1000> toMillis() { return {}; }
};

template <uint64_t value>
struct Milliseconds: public LiteralTimeUnit<value, ::Time::Milliseconds, Milliseconds> {
	typedef LiteralTimeUnit<value, ::Time::Milliseconds, Milliseconds> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    template <typename prescaled_t>
    static constexpr auto toCountsOn() {
        constexpr Counts<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / 1000> result = {};
        static_assert(result.getValue() > 1,
                "Number of counts for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr auto toTicksOn() {
        constexpr Ticks<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / 1000 / (uint64_t(prescaled_t::maximum) + 1)> result = {};
        static_assert(result.getValue() > 1,
                "Number of ticks for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr Microseconds<value * 1000> toMicrosOn() { return {}; }

    static constexpr Microseconds<value * 1000> toMicros() { return {}; }

    template <typename prescaled_t>
    static constexpr Milliseconds<value> toMillisOn() { return {}; }

    static constexpr Milliseconds<value> toMillis() { return {}; }
};

template <uint64_t value>
struct Seconds: public LiteralTimeUnit<value, ::Time::Seconds, Seconds> {
	typedef LiteralTimeUnit<value, ::Time::Seconds, Seconds> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    template <typename prescaled_t>
    static constexpr auto toCountsOn() {
        constexpr Counts<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value> result = {};
        static_assert(result.getValue() > 1,
                "Number of counts for seconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr auto toTicksOn() {
        constexpr Ticks<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / (uint64_t(prescaled_t::maximum) + 1)> result = {};
        static_assert(result.getValue() > 1,
                "Number of ticks for seconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr Microseconds<value * 1000000> toMicrosOn() { return {}; }

    static constexpr Microseconds<value * 1000000> toMicros() { return {}; }

    template <typename prescaled_t>
    static constexpr Milliseconds<value * 1000> toMillisOn() { return {}; }

    static constexpr Milliseconds<value * 1000> toMillis() { return {}; }
};

template <uint64_t value>
struct Minutes: public LiteralTimeUnit<value, ::Time::Minutes, Minutes> {
	typedef LiteralTimeUnit<value, ::Time::Minutes, Minutes> Super;
	using Super::instance;
	using Super::percent;
	using Super::times;

    template <typename prescaled_t>
    static constexpr auto toCountsOn() {
        constexpr Counts<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value * 60> result = {};
        static_assert(result.getValue() > 1,
                "Number of counts for minutes is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr auto toTicksOn() {
        constexpr Ticks<(uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value * 60 / (uint64_t(prescaled_t::maximum) + 1)> result = {};
        static_assert(result.getValue() > 1,
                "Number of ticks for minutes is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <typename prescaled_t>
    static constexpr Microseconds<value * 60000000> toMicrosOn() { return {}; }

    static constexpr Microseconds<value * 60000000> toMicros() { return {}; }

    template <typename prescaled_t>
    static constexpr Milliseconds<value * 60000> toMillisOn() { return {}; }

    static constexpr Milliseconds<value * 60000> toMillis() { return {}; }
};

template <typename t>
constexpr t pow(t base, int exp) {
  return (exp > 0) ? base * pow(base, exp-1) : 1;
}

template <char...> struct literal;
template <> struct literal<> {
  static constexpr uint64_t to_uint64 = 0;
};
template <char c, char ...cv> struct literal<c, cv...> {
  static constexpr uint64_t to_uint64 = (c - '0') * pow(uint64_t(10), sizeof...(cv)) + literal<cv...>::to_uint64;
};

}

template <uint32_t value>
bool constexpr operator== (Ticks a, Lit::Ticks<value> b) { return a.getValue() == b; }

template <uint64_t value>
inline void delay(Lit::Microseconds<value> us) {
    static_assert(value < 768.0 / (F_CPU / 1000000.0), "Busy wait delay has max 768us / F_CPU_MHZ. Consider using RealTimer::delay instead.");
    _delay_us(value);
}

template <char ...cv>
constexpr Lit::PercentApply<Lit::literal<cv...>::to_uint64> operator "" _percentOf() { return {}; }

template <char ...cv>
constexpr Lit::Counts<Lit::literal<cv...>::to_uint64> operator "" _counts() { return {}; }

template <char ...cv>
constexpr Lit::Ticks<Lit::literal<cv...>::to_uint64> operator "" _ticks() { return {}; }

template <char ...cv>
constexpr Lit::Milliseconds<Lit::literal<cv...>::to_uint64> operator "" _ms() { return {}; }

template <char ...cv>
constexpr Lit::Microseconds<Lit::literal<cv...>::to_uint64> operator "" _us() { return {}; }

template <char ...cv>
constexpr Lit::Seconds<Lit::literal<cv...>::to_uint64> operator "" _s() { return {}; }

template <char ...cv>
constexpr Lit::Seconds<Lit::literal<cv...>::to_uint64> operator "" _sec() { return {}; }

template <char ...cv>
constexpr Lit::Minutes<Lit::literal<cv...>::to_uint64> operator "" _min() { return {}; }

template <uint64_t divisor, typename time_t>
constexpr auto dividedBy(time_t t) -> decltype(time_t::template dividedBy<divisor>()) { return {}; }

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant takes references for both the prescaled timer and the time unit.
 * Typically invoked like:
 *
 *     auto counts = toCounts(timer1, 200_ms); // returns Counts<n>
 */
template <typename prescaled_t, typename duration_t>
constexpr auto toCountsOn(const prescaled_t &, const duration_t duration) {
	return duration_t::template toCountsOn<prescaled_t>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take a type for the prescaled timer, and a reference for the time unit.
 * Typically invoked like:
 *
 *     uint16_t counts = toCounts<timer1_t>(200_ms);
 */
template <typename prescaled_t, typename duration_t>
constexpr auto toCountsOn(const duration_t duration) {
	return duration.template toCountsOn<prescaled_t>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take types for both the prescaled timer, and the time unit.
 * Typically invoked like:
 *
 *     typedef decltype(200_ms) duration_t;
 *     uint16_t counts = toCounts<timer1_t, duration_t>();
 */
template <typename prescaled_t, typename duration_t>
constexpr auto toCountsOn() {
    return duration_t::template toCountsOn<prescaled_t>();
}

//---- see above for documentation
template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn(const prescaled_t &, const duration_t duration) {
	return duration_t::template toTicksOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn(const duration_t duration) {
	return duration.template toTicksOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn() {
    return duration_t::template toTicksOn<prescaled_t>();
}

template <typename prescaled_t, typename duration_t>
constexpr auto toMillisOn(const prescaled_t &, const duration_t duration) {
	return duration_t::template toMillisOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toMillisOn(const duration_t duration) {
	return duration.template toMillisOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toMillisOn() {
    return duration_t::template toMillisOn<prescaled_t>();
}

template <typename prescaled_t, typename duration_t>
constexpr auto toMicrosOn(const prescaled_t &, const duration_t duration) {
	return duration_t::template toMicrosOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toMicrosOn(const duration_t duration) {
	return duration.template toMicrosOn<prescaled_t>();
}
template <typename prescaled_t, typename duration_t>
constexpr auto toMicrosOn() {
    return duration_t::template toMicrosOn<prescaled_t>();
}

}


