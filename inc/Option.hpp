#pragma once

template <typename T>
class Option;

class None {};

template <typename T>
class Option {
	template <typename T2>
	friend Option<T2> none();

	bool present;
	T value;
public:
	constexpr Option(None const &): present(false), value(0) {}
	constexpr Option(const T t): present(true), value(t) {}
	bool isDefined() const { return present; }
	bool isEmpty() const { return !present; }
	template <typename l_t>
	inline __attribute__((always_inline)) void forEach(l_t l) const { if (present) l(value); }
};

None none() {
	return None();
}


template <typename T>
Option<T> some(const T t) {
	return Option<T>(t);
}

