#ifndef TIMEUNITS_H
#define TIMEUNITS_H

#include <stdint.h>

namespace Time {

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
    MultipliedTimeUnit() {
        static_assert(percentage > 0, "percentage must be larger than zero");
    }
    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = Base::to_uint64>
    static constexpr return_t toCounts() {
        return Base::template toCounts<prescaled_t, return_t, uint64_t(value) * percentage / 100>();
    }
    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = Base::to_uint64>
    static constexpr return_t toTicks() {
        return Base::template toTicks<prescaled_t, return_t, uint64_t(value) * percentage / 100>();
    }
};



template <char... cv>
class Counts: public TimeUnit<cv...> {
public:
    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toCounts() {
        static_assert(value <= std::numeric_limits<return_t>::max(),
                "Number of counts does not fit in return_t, you might want to widen the return type.");
        return value;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr typename prescaled_t::value_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, prescaled_t::value_t, value>();
    }

    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toTicks() {
        //return prescaled_t::template microseconds2ticks<value,return_t>();
        constexpr auto maximum = prescaled_t::maximum;

        static_assert(value / (maximum + 1) > 1,
                "Number of ticks for counts is so low that it rounds to 0 or 1, you shouldn't use this counts value as ticks.");
        static_assert(value / (maximum + 1) <= std::numeric_limits<return_t>::max(),
                "Number of ticks for counts does not fit in return_t, you might want to widen the return type.");
        return value / (maximum + 1);
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Counts<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Counts<cv...>,percentage>();
    }
};

template <char ...cv>
constexpr Counts<cv...> operator "" _counts() { return Counts<cv...>(); }



template <char... cv>
class Microseconds: public TimeUnit<cv...> {
public:
    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toCounts() {
        constexpr auto power2 = prescaled_t::prescalerPower2;

        static_assert((uint64_t(F_CPU) >> power2) / 1000 * value / 1000 > 1,
                "Number of counts for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        static_assert((uint64_t(F_CPU) >> power2) / 1000 * value / 1000 <= std::numeric_limits<return_t>::max(),
                "Number of counts for microseconds does not fit in return_t, you might want to increase the timer prescaler or widen the return type.");
        return (F_CPU >> power2) / 1000 * value / 1000;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr typename prescaled_t::value_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, prescaled_t::value_t, value>();
    }

    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toTicks() {
        constexpr auto power2 = prescaled_t::prescalerPower2;
        constexpr auto maximum = prescaled_t::maximum;

        static_assert((uint64_t(F_CPU) >> power2) / 1000 / (maximum + 1) * value / 1000 > 1,
                "Number of ticks for microseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        static_assert((uint64_t(F_CPU) >> power2) / 1000 / (maximum + 1) * value / 1000 <= std::numeric_limits<return_t>::max(),
                "Number of ticks for microseconds does not fit in return_t, you might want to increase the timer prescaler or widen the return type.");
        return (F_CPU >> power2) / 1000 / (maximum + 1) * value / 1000;
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
    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toCounts() {
        constexpr auto power2 = prescaled_t::prescalerPower2;

        static_assert((uint64_t(F_CPU) >> power2) / 1000 * value > 1,
                "Number of counts for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        static_assert((uint64_t(F_CPU) >> power2) / 1000 * value <= std::numeric_limits<return_t>::max(),
                "Number of counts for milliseconds does not fit in return_t, you might want to increase the timer prescaler or widen the return type.");
        return (F_CPU >> power2) / 1000 * value;
    }

    template <uint64_t value = TimeUnit<cv...>::to_uint64, typename prescaled_t>
    static constexpr typename prescaled_t::value_t toCounts(const prescaled_t &) {
        return toCounts<prescaled_t, prescaled_t::value_t, value>();
    }

    template <typename prescaled_t, typename return_t = typename prescaled_t::value_t, uint64_t value = TimeUnit<cv...>::to_uint64>
    static constexpr return_t toTicks() {
        constexpr auto power2 = prescaled_t::prescalerPower2;
        constexpr auto maximum = prescaled_t::maximum;

        static_assert((uint64_t(F_CPU) >> power2) / 1000 / (maximum + 1) * value > 1,
                "Number of ticks for milliseconds is so low that it rounds to 0 or 1, you might want to decrease the timer prescaler.");
        static_assert((uint64_t(F_CPU) >> power2) / 1000 / (maximum + 1) * value <= std::numeric_limits<return_t>::max(),
                "Number of ticks for milliseconds does not fit in return_t, you might want to increase the timer prescaler or widen the return type.");
        return (F_CPU >> power2) / 1000 * value;
    }

    template <int percentage>
    static constexpr MultipliedTimeUnit<Milliseconds<cv...>,percentage> percent() {
        return MultipliedTimeUnit<Milliseconds<cv...>,percentage>();
    }
};

template <char ...cv>
constexpr Milliseconds<cv...> operator "" _ms() { return Milliseconds<cv...>(); }


/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant takes references for both the prescaled timer and the time unit.
 * Typically invoked like:
 *
 *     uint16_t counts = toCountsOn(timer1, 200_ms);
 */
template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toCountsOn(const prescaled_t &, const duration_t) {
    return duration_t::template toCounts<prescaled_t, return_t>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take a type for the prescaled timer, and a reference for the time unit.
 * Typically invoked like:
 *
 *     uint16_t counts = toCountsOn<timer1_t>(200_ms);
 */
template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toCountsOn(const duration_t) {
    return duration_t::template toCounts<prescaled_t, return_t>();
}

/**
 * Converts the given time unit duration to timer counts, when running on the given prescaled timer.
 * This variant take types for both the prescaled timer, and the time unit.
 * Typically invoked like:
 *
 *     typedef decltype(200_ms) duration_t;
 *     uint16_t counts = toCountsOn<timer1_t, duration_t>();
 */
template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toCountsOn() {
    return duration_t::template toCounts<prescaled_t, return_t>();
}

template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toTicksOn(const prescaled_t &, const duration_t) {
    return duration_t::template toTicks<prescaled_t, return_t>();
}

template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toTicksOn(const duration_t) {
    return duration_t::template toTicks<prescaled_t, return_t>();
}

template <typename prescaled_t, typename duration_t, typename return_t = typename prescaled_t::value_t>
constexpr return_t toTicksOn() {
    return duration_t::template toTicks<prescaled_t, return_t>();
}

}

#endif
