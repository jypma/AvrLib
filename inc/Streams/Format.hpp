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

template <typename T>
struct Decimal {
    T value;
};

template <typename T>
inline Decimal<T> dec(T v) {
    return Decimal<T> { v };
}

class Format {
public:
    typedef void (write_f)(void *ctx, uint8_t value);

    static void format(write_f write, void *ctx, Decimal<uint8_t> v);
    static void format(write_f write, void *ctx, Decimal<int8_t> v);
    static void format(write_f write, void *ctx, Decimal<uint16_t> v);
    static void format(write_f write, void *ctx, Decimal<int16_t> v);
    static void format(write_f write, void *ctx, Decimal<uint32_t> v);
    static void format(write_f write, void *ctx, Decimal<int32_t> v);
};

}

#endif /* FORMAT_HPP_ */
