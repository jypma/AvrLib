#pragma once

#include "gcc_type_traits.h"
#include <stdint.h>

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
class OptionIter {
    bool present;
    T value;
public:
    constexpr OptionIter(bool p, T t): present(p), value(t) {}
    constexpr OptionIter(): present(false) {}
    constexpr T operator*() const { return value; }

    constexpr bool operator!= (const OptionIter<T> that) const {
        return (!present && that.present) || (present && (!that.present || value != that.value));
    }

    void operator++() {
        present = false;
    }
};

template <typename T>
class Option {
	template <typename T2>
	friend Option<T2> none();

	template <typename T2>
	friend class Option;

	bool present;
	struct Placeholder {
        uint8_t _[sizeof(T)];
    };
    Placeholder memory_;
    const T* memory() const { return reinterpret_cast<const T*>(&memory_); }
    T* memory() { return reinterpret_cast<T*>(&memory_); }
public:
	constexpr Option(): present(false) {}
	constexpr Option(None const &): present(false) {}
	constexpr Option(const T t): present(true) { *memory() = t; }
	constexpr bool isDefined() const { return present; }
	constexpr bool isEmpty() const { return !present; }
    constexpr T get() const { return *(memory()); }

	template <typename l_t>
	constexpr inline __attribute__((always_inline)) void forEach(l_t l) const { if (present) l(get()); }

	constexpr const OptionIter<T> begin() const { return { present, get() }; }
	constexpr const OptionIter<T> end() const { return { }; }

	template <typename l_t>
	constexpr inline __attribute__((always_inline)) Option<typename std::result_of<l_t(T)>::type> map(l_t l) const {
		if (present) return l(get()); else return NONE;
	}

	constexpr inline __attribute__((always_inline)) T getOrElse(T alternative) const {
		return (present) ? get() : alternative;
	}

	template <typename T2>
	constexpr bool operator== (const Option<T2> that) const {
		return (!present && !that.present) || (present && that.present && get() == that.get());
	}

    template <typename T2>
    constexpr bool operator!= (const Option<T2> that) const {
        return (!present && that.present) || (present && (!that.present || get() != that.get()));
    }

	template <typename T2>
	constexpr operator Option<T2>() {
		return (present) ? Option<T2>(get()) : Option<T2>(NONE);
	}

	template <typename T2> void operator /= (const T2 t2) { if (present) *(memory()) /= t2; }
	template <typename T2> void operator += (const T2 t2) { if (present) *(memory()) += t2; }
	template <typename T2> void operator -= (const T2 t2) { if (present) *(memory()) -= t2; }
	template <typename T2> void operator *= (const T2 t2) { if (present) *(memory()) *= t2; }

	template <typename T2> constexpr bool operator < (const T2 t2) { return present && get() < t2; }
	template <typename T2> constexpr bool operator <= (const T2 t2) { return present && get() <= t2; }
	template <typename T2> constexpr bool operator > (const T2 t2) { return present && get() > t2; }
	template <typename T2> constexpr bool operator >= (const T2 t2) { return present && get() >= t2; }

	template <typename T2> constexpr auto operator + (const T2 t2) -> Option<decltype(get() + t2)> {
		if (present) return (get() + t2); else return NONE;
	}
	template <typename T2> constexpr auto operator - (const T2 t2) -> Option<decltype(get() - t2)> {
		if (present) return (get() - t2); else return NONE;
	}
	template <typename T2> constexpr auto operator * (const T2 t2) -> Option<decltype(get() * t2)> {
		if (present) return (get() * t2); else return NONE;
	}
	template <typename T2> constexpr auto operator / (const T2 t2) -> Option<decltype(get() / t2)> {
		if (present) return (get() / t2); else return NONE;
	}
};



template <typename T>
constexpr Option<T> some(const T t) {
	return Option<T>(t);
}

