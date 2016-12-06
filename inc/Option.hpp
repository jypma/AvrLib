#pragma once

#include "gcc_type_traits.h"

template <typename T>
class Option;

class None {
public:
	template <typename T>
	constexpr bool operator ==(const Option<T> t) const { return t.isEmpty(); }
};
constexpr None NONE = None();

inline constexpr None none() {
	return NONE;
}

template <typename T>
class Option {
	template <typename T2>
	friend Option<T2> none();

	template <typename T2>
	friend class Option;

	bool present;
	T value;
public:
	constexpr Option(): present(false), value(0) {}
	constexpr Option(None const &): present(false), value(0) {}
	constexpr Option(const T t): present(true), value(t) {}
	constexpr bool isDefined() const { return present; }
	constexpr bool isEmpty() const { return !present; }

	template <typename l_t>
	constexpr inline __attribute__((always_inline)) void forEach(l_t l) const { if (present) l(value); }

	template <typename l_t>
	constexpr inline __attribute__((always_inline)) Option<typename std::result_of<l_t(T)>::type> map(l_t l) const {
		if (present) return l(value); else return NONE;
	}

	constexpr T get() const { return value; }

	constexpr inline __attribute__((always_inline)) T getOrElse(T alternative) const {
		return (present) ? value : alternative;
	}

	template <typename T2>
	constexpr bool operator== (const Option<T2> that) const {
		return (!present && !that.present) || (present && that.present && value == that.value);
	}

	template <typename T2> void operator /= (const T2 t2) { if (present) value /= t2; }
	template <typename T2> void operator += (const T2 t2) { if (present) value += t2; }
	template <typename T2> void operator -= (const T2 t2) { if (present) value -= t2; }
	template <typename T2> void operator *= (const T2 t2) { if (present) value *= t2; }
	template <typename T2> constexpr bool operator < (const T2 t2) { return present && value < t2; }
	template <typename T2> constexpr bool operator <= (const T2 t2) { return present && value <= t2; }
	template <typename T2> constexpr bool operator > (const T2 t2) { return present && value > t2; }
	template <typename T2> constexpr bool operator >= (const T2 t2) { return present && value >= t2; }
};



template <typename T>
Option<T> some(const T t) {
	return Option<T>(t);
}

