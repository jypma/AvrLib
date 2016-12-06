#pragma once

#include <avr/io.h>

// We don't want the log() function in the global namespace, just because we're using _delay_ms
#ifndef __MATH_H
#define __MATH_H
extern "C" {
extern double fabs(double __x) __attribute__((__const__));
extern double ceil(double __x) __attribute__((__const__));
}
#define redef_math
#endif
#include <util/delay.h>
#ifdef redef_math
#undef __MATH_H
#endif

#include <stdint.h>

namespace Time {

template<typename This>
class RuntimeTimeUnit {
protected:
    uint32_t value;
public:
    constexpr RuntimeTimeUnit(uint32_t v): value(v) {}
    constexpr uint32_t getValue() const { return value; }
    constexpr operator uint32_t() const { return value; }
    constexpr bool operator== (const This that) const { return that.value == value; }
    constexpr bool operator< (const This that) const { return value < that.value; }
    constexpr bool operator<= (const This that) const { return value <= that.value; }
    constexpr bool operator> (const This that) const { return value > that.value; }
    constexpr bool operator>= (const This that) const { return value >= that.value; }
};

class Counts;
class Ticks;
class Milliseconds;
class Microseconds;
class Seconds;
class Minutes;

/**
 * Counts represent individual timer increments.
 */
class Counts: public RuntimeTimeUnit<Counts> {
    using RuntimeTimeUnit<Counts>::RuntimeTimeUnit;
public:
    template <typename prescaled_t>
    constexpr Counts toCounts() const { return *this; }

    template <typename prescaled_t>
    constexpr Milliseconds toMillis() const;

    template <typename prescaled_t>
    constexpr Microseconds toMicros() const;
};

/**
 * Ticks represent timer overflows, i.e. every 256th or 65kth count, depending on timer size.
 */
class Ticks: public RuntimeTimeUnit<Ticks> {
    using RuntimeTimeUnit<Ticks>::RuntimeTimeUnit;
public:
    template <typename prescaled_t>
    constexpr Milliseconds toMillis() const;

    template <typename prescaled_t>
    constexpr Counts toCounts() const {
    	return Counts(getValue() * (prescaled_t::maximum + 1));
    }
};


class Microseconds: public RuntimeTimeUnit<Microseconds> {
    using RuntimeTimeUnit<Microseconds>::RuntimeTimeUnit;
public:
    template <typename prescaled_t>
    constexpr Ticks toTicks() const {
        constexpr float countsPerUs = (uint64_t(F_CPU / 1000) >> prescaled_t::prescalerPower2) / 1000.0;
        constexpr float ticksPerUs = countsPerUs / float(prescaled_t::maximum + 1);
        constexpr float max = 0xFFFFFFFF / ticksPerUs;

        const float v = getValue();
        return (v >= max) ? 0xFFFFFFFF : v * ticksPerUs;
    }

    template <typename prescaled_t>
    constexpr Milliseconds toMillis() const;
};

template <typename prescaled_t>
constexpr Microseconds Counts::toMicros() const {
    constexpr float countsPerUs = float(uint64_t(F_CPU) / 1000) / (1 << prescaled_t::prescalerPower2) / 1000;
    constexpr float max = 0xFFFFFFFF * countsPerUs;

    const float v = getValue();
    return (v >= max) ? 0xFFFFFFFF : v / countsPerUs;
}

class Milliseconds: public RuntimeTimeUnit<Milliseconds> {
    using RuntimeTimeUnit<Milliseconds>::RuntimeTimeUnit;
public:
    using RuntimeTimeUnit::operator<;
    using RuntimeTimeUnit::operator<=;
    using RuntimeTimeUnit::operator>;
    using RuntimeTimeUnit::operator>=;
    using RuntimeTimeUnit::operator==;

    template <typename prescaled_t>
    constexpr Ticks toTicks() const {
        constexpr float countsPerMs = uint64_t(F_CPU / 1000) >> prescaled_t::prescalerPower2;
        constexpr float ticksPerMs = float(countsPerMs) / (uint64_t(prescaled_t::maximum) + 1);
        constexpr float max = 0xFFFFFFFF / ticksPerMs;

        const float v = getValue();
        return (v >= max) ? 0xFFFFFFFF : v * ticksPerMs;
    }

    template <typename prescaled_t>
    constexpr Milliseconds toMillis() const {
    	return *this;
    }

    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    constexpr bool operator> (const time_t that) const { return value > that.toMillis(); }
    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    constexpr bool operator< (const time_t that) const { return value < that.toMillis(); }
    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    constexpr bool operator== (const time_t that) const { return value == that.toMillis(); }
    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    constexpr bool operator>= (const time_t that) const { return value >= that.toMillis(); }
    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    constexpr bool operator<= (const time_t that) const { return value <= that.toMillis(); }
    template <typename time_t, typename check=decltype(&time_t::toMillis)>
    void operator= (const time_t that) { value = that.toMillis(); }
};

template <typename prescaled_t>
constexpr Milliseconds Counts::toMillis() const {
    constexpr float countsPerMs = float(uint64_t(F_CPU) / 1000) / (1 << prescaled_t::prescalerPower2);
    constexpr float max = 0xFFFFFFFF * countsPerMs;

    const float v = getValue();
    return (v >= max) ? 0xFFFFFFFF : v / countsPerMs;
}

template <typename prescaled_t>
constexpr Milliseconds Ticks::toMillis() const {
    constexpr float ticksPerMs = float(uint64_t(F_CPU) / 1000) / (1 << prescaled_t::prescalerPower2) / (uint64_t(prescaled_t::maximum) + 1);
    constexpr float max = 0xFFFFFFFF * ticksPerMs;

    const float v = getValue();
    return (v >= max) ? 0xFFFFFFFF : v / ticksPerMs;
}

template <typename prescaled_t>
constexpr Milliseconds Microseconds::toMillis() const {
	return getValue() / 1000;
}

class Seconds: public RuntimeTimeUnit<Seconds> {
    using RuntimeTimeUnit<Seconds>::RuntimeTimeUnit;
};

class Minutes: public RuntimeTimeUnit<Minutes> {
    using RuntimeTimeUnit<Minutes>::RuntimeTimeUnit;
};

}

