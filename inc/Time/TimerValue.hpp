#pragma once

#include "RuntimeUnits.hpp"
#include "UnitLiterals.hpp"
#include "Counting.hpp"

namespace Time {

template <typename prescaled_t>
class TimerValue {
public:
	typedef TimerValue<prescaled_t> This;
	typedef typename prescaled_t::value_t value_t;
private:
	value_t value;
public:
	constexpr static This maximum() { return Counting<value_t>::maximum; }

	constexpr TimerValue(): value(0) {}
	constexpr TimerValue(value_t v): value(v) {}
	constexpr TimerValue(const TimerValue<prescaled_t> &v): value(v.value) {}
	constexpr TimerValue(const volatile TimerValue<prescaled_t> &v): value(v.value) {}

	template <uint64_t v>
	constexpr TimerValue(::Time::Lit::Counts<v> counts): value(counts.getValue()) {}

	void operator= (value_t v) volatile { value = v; }
	void operator= (const This &v) volatile { value = v.value; }
	void operator= (const volatile This &v) volatile { value = v.value; }

	constexpr bool operator> (const This &that) const volatile { return value > that.value; }
	constexpr bool operator> (const volatile This &that) const volatile { return value > that.value; }
	template <uint64_t v> constexpr This operator- (Lit::Counts<v>) const volatile { return value - v; }
	constexpr This operator- (const This &that) const volatile { return value - that.value; }
	constexpr This operator- (const volatile This &that) const volatile { return value - that.value; }

	constexpr value_t getValue() const volatile { return value; }
	constexpr operator value_t() const volatile { return value; }
};

}


