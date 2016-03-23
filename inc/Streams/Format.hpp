/*
 * Format.hpp
 *
 *  Created on: Jun 4, 2015
 *      Author: jan
 */

#ifndef FORMAT_HPP_
#define FORMAT_HPP_

#include <stdint.h>

namespace Streams {
namespace Impl {

template <typename T>
struct Decimal {
    const T value;
    constexpr explicit Decimal(T v): value(v) {}
};

template <typename T>
struct Hexadecimal {
    const T value;
    constexpr explicit Hexadecimal(T v): value(v) {}
};

template <typename int_t>
struct ArraySlice {
    const int_t *array;
    uint8_t first;
    uint8_t until;
};

class Format {
public:
    typedef bool (write_f)(void *ctx, uint8_t value);

    static bool format(write_f write, void *ctx, Impl::Decimal<uint8_t> v);
    static bool format(write_f write, void *ctx, Impl::Decimal<int8_t> v);
    static bool format(write_f write, void *ctx, Impl::Decimal<uint16_t> v);
    static bool format(write_f write, void *ctx, Impl::Decimal<int16_t> v);
    static bool format(write_f write, void *ctx, Impl::Decimal<uint32_t> v);
    static bool format(write_f write, void *ctx, Impl::Decimal<int32_t> v);
};

}

// FIXME rename to Decimal and bump class to Impl namespace
template <typename T>
inline Impl::Decimal<T> constexpr dec(T v) {
    return Impl::Decimal<T> { v };
}

template <typename T>
inline Impl::Decimal<T> constexpr Decimal(T v) {
    return Impl::Decimal<T> { v };
}

template <typename int_t>
inline Impl::Decimal<Impl::ArraySlice<int_t>> constexpr Decimal(const int_t *array, uint8_t first, uint8_t count) {
    return Impl::Decimal<Impl::ArraySlice<int_t>> { { array, first, uint8_t(first + count) } };
}

template <typename T>
inline Impl::Hexadecimal<T> constexpr Hexadecimal(T v) {
    return Impl::Hexadecimal<T> { v };
}

}

#endif /* FORMAT_HPP_ */
