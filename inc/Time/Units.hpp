#ifndef TIMEUNITS_H
#define TIMEUNITS_H

#include <avr/io.h>
#include <stdint.h>

namespace Time {

template<typename This>
class RuntimeTimeUnit {
    uint32_t value;
public:
    constexpr RuntimeTimeUnit(uint32_t v): value(v) {}
    constexpr uint32_t getValue() const { return value; }
    constexpr bool operator== (const This that) const { return that.value == value; }
};

template <char... cv> class Milliseconds;

template <typename T, T value>
class Truncatable {
    template <typename I>
    static constexpr bool isValid() {
        return value <= std::numeric_limits<I>::max();
    }

public:
    template <typename I>
    constexpr operator I() {
        static_assert(isValid<I>(),
                "A value does not fit in return_t, you might want to widen the return type, or in case of a timed value, increase the timer prescaler.");
        return value;
    }

    static constexpr bool is_uint8 = isValid<uint8_t>();
    static constexpr bool is_uint16 = isValid<uint16_t>();
    static constexpr bool is_uint32 = isValid<uint32_t>();
};

template <uint64_t value> using ut64_t = Truncatable<uint64_t, value>;

template <typename t>
constexpr t pow(t base, int exp) {
  return (exp > 0) ? base * pow(base, exp-1) : 1;
};

template <char...> struct literal;
template <> struct literal<> {
  static constexpr uint64_t to_uint64 = 0;
};
template <char c, char ...cv> struct literal<c, cv...> {
  static constexpr uint64_t to_uint64 = (c - '0') * pow(uint64_t(10), sizeof...(cv)) + literal<cv...>::to_uint64;
};

template<typename Base, int percentage>
class MultipliedTimeUnit;

template <char... cv>
class TimeUnit {
    typedef literal<cv...> value;
public:
    static constexpr uint64_t to_uint64 = value::to_uint64;
};

template<typename Base, int percentage>
class MultipliedTimeUnit {
public:
    static constexpr MultipliedTimeUnit<Base, percentage> instance = MultipliedTimeUnit();

    MultipliedTimeUnit() {
        static_assert(percentage > 0, "percentage must be larger than zero");
    }
    template <typename prescaled_t, uint64_t value = Base::to_uint64>
    static constexpr uint64_t toCounts() {
        return Base::template toCounts<prescaled_t, uint64_t(value) * percentage / 100>();
    }
    template <typename prescaled_t, uint64_t value = Base::to_uint64>
    static constexpr uint64_t toTicks() {
        return Base::template toTicks<prescaled_t, uint64_t(value) * percentage / 100>();
    }
};

/**
 * Counts represent individual timer increments.
 */
template <char... cv>
class Counts: public TimeUnit<cv...> {
public:
    static constexpr Counts<cv...> instance = Counts();

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toCounts() {
        return value;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr uint64_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, prescaled_t::value_t, value>();
    }

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toTicks() {
        constexpr uint64_t result = value / (prescaled_t::maximum + 1);
        static_assert(result > 1,
                "Number of ticks for counts is so low that it rounds to 0 or 1, you shouldn't use this counts value as ticks.");
        return result;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Counts<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Counts<cv...>,percentage>();
    }
};

template<>
class Counts<>: public RuntimeTimeUnit<Counts<>> {
    using RuntimeTimeUnit<Counts<>>::RuntimeTimeUnit;
public:
    template <typename prescaled_t>
    constexpr Milliseconds<> toMillisOn() const;
};

template <char ...cv>
constexpr Counts<cv...> operator "" _counts() { return Counts<cv...>(); }

/**
 * Ticks represent timer overflows, i.e. every 256th or 65kth count, depending on timer size.
 */
template<char... cv>
class Ticks: public TimeUnit<cv...> {
    // TODO
};

template<>
class Ticks<>: public RuntimeTimeUnit<Ticks<>> {
    using RuntimeTimeUnit<Ticks<>>::RuntimeTimeUnit;
};

template <char... cv>
class Microseconds: public TimeUnit<cv...> {
public:
    static constexpr Microseconds<cv...> instance = Microseconds();

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toCounts() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) / 1000 * value / 1000;
        static_assert(result > 1,
                "Number of counts for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr uint64_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, value>();
    }

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toTicks() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) / 1000 / (prescaled_t::maximum + 1) * value / 1000;
        static_assert(result > 1,
                "Number of ticks for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Microseconds<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Microseconds<cv...>,percentage>();
    }
};

template <char ...cv>
constexpr Microseconds<cv...> operator "" _us() { return Microseconds<cv...>(); }


template <char... cv>
class Milliseconds: public TimeUnit<cv...> {
public:
    static constexpr Milliseconds<cv...> instance = Milliseconds();

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toCounts() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / 1000;
        static_assert(result > 1,
                "Number of counts for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr uint64_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, value>();
    }

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toTicks() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / 1000 / (prescaled_t::maximum + 1);
        static_assert(result > 1,
                "Number of ticks for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Milliseconds<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Milliseconds<cv...>,percentage>();
    }
};

template<>
class Milliseconds<>: public RuntimeTimeUnit<Milliseconds<>> {
    using RuntimeTimeUnit<Milliseconds<>>::RuntimeTimeUnit;
public:
    template <typename prescaled_t>
    constexpr Ticks<> toTicksOn() const {
        constexpr float countsPerMs = uint64_t(F_CPU / 1000) >> prescaled_t::prescalerPower2;
        constexpr float ticksPerMs = countsPerMs / float(prescaled_t::maximum + 1);
        constexpr float max = 0xFFFFFFFF / ticksPerMs;

        const float v = getValue();
        return (v >= max) ? 0xFFFFFFFF : v * ticksPerMs;
    }
};

template <char ...cv>
constexpr Milliseconds<cv...> operator "" _ms() { return Milliseconds<cv...>(); }

template <typename prescaled_t>
constexpr Milliseconds<> Counts<>::toMillisOn() const {
    constexpr float countsPerMs = float(uint64_t(F_CPU) / 1000) / (1 << prescaled_t::prescalerPower2);
    constexpr float max = 0xFFFFFFFF * countsPerMs;

    const float v = getValue();
    return (v >= max) ? 0xFFFFFFFF : v / countsPerMs;
}


template <char... cv>
class Seconds: public TimeUnit<cv...> {
public:
    static constexpr Seconds<cv...> instance = Seconds();

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toCounts() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value;
        static_assert(result > 1,
                "Number of counts for seconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr uint64_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, value>();
    }

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toTicks() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value / (prescaled_t::maximum  + 1) ;
        static_assert(result > 1,
                "Number of ticks for seconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Seconds<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Seconds<cv...>,percentage>();
    }
};

template <char ...cv>
constexpr Seconds<cv...> operator "" _s() { return Seconds<cv...>(); }


template <char... cv>
class Minutes: public TimeUnit<cv...> {
public:
    static constexpr Minutes<cv...> instance = Minutes();

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toCounts() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value * 60;
        static_assert(result > 1,
                "Number of counts for minutes is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr uint64_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, value>();
    }

    template <typename prescaled_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr uint64_t toTicks() {
        constexpr auto result = (uint64_t(F_CPU) >> prescaled_t::prescalerPower2) * value * 60 / (prescaled_t::maximum  + 1) ;
        static_assert(result > 1,
                "Number of ticks for minutes is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        return result;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Minutes<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Minutes<cv...>,percentage>();
    }
};

template <char ...cv>
constexpr Minutes<cv...> operator "" _min() { return Minutes<cv...>(); }

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant takes references for both the prescaled timer and the time unit.
 * Typically invoked like:
 *
 *     uint16_t counts = toCountsOn(timer1, 200_ms);
 */
template <typename prescaled_t, typename duration_t>
constexpr ut64_t<duration_t::template toCounts<prescaled_t>()> toCountsOn(const prescaled_t &, const duration_t) {
    return ut64_t<duration_t::template toCounts<prescaled_t>()>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take a type for the prescaled timer, and a reference for the time unit.
 * Typically invoked like:
 *
 *     uint16_t counts = toCountsOn<timer1_t>(200_ms);
 */
template <typename prescaled_t, typename duration_t>
constexpr ut64_t<duration_t::template toCounts<prescaled_t>()> toCountsOn(const duration_t) {
    return ut64_t<duration_t::template toCounts<prescaled_t>()>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take types for both the prescaled timer, and the time unit.
 * Typically invoked like:
 *
 *     typedef decltype(200_ms) duration_t;
 *     uint16_t counts = toCountsOn<timer1_t, duration_t>();
 */
template <typename prescaled_t, typename duration_t>
constexpr ut64_t<duration_t::template toCounts<prescaled_t>()> toCountsOn() {
    return ut64_t<duration_t::template toCounts<prescaled_t>()>();
}

template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn(const prescaled_t &, const duration_t) {
    return ut64_t<duration_t::template toTicks<prescaled_t>()>();
}

template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn(const duration_t) {
    return ut64_t<duration_t::template toTicks<prescaled_t>()>();
}

template <typename prescaled_t>
constexpr Ticks<> toTicksOn(Milliseconds<> time) {
    return time.template toTicksOn<prescaled_t>();
}

template <typename prescaled_t, typename duration_t>
constexpr auto toTicksOn() {
    return ut64_t<duration_t::template toTicks<prescaled_t>()>();
}

template <typename prescaled_t, typename time_t>
Milliseconds<> toMillisOn(const time_t time) {
    return time.template toMillisOn<prescaled_t>();
}

}

#endif
