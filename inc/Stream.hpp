/*
 * Stream.hpp
 *
 *  Created on: Jan 15, 2015
 *      Author: jan
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include <avr/common.h>

template <typename T>
struct Decimal {
    T value;
};

inline Decimal<uint8_t> dec(uint8_t v) {
    return Decimal<uint8_t> { v };
}

class Stream {
    void (*const writeFunc)(const uint8_t ch);
public:
    constexpr Stream(void (*_write)(const uint8_t ch)): writeFunc(_write) {}

    const Stream &operator << (const char *string) const {
        if (string != nullptr) {
            char c = *string;
            while (c) {
                writeFunc(c);
                string++;
                c = *string;
            }
        }

        return *this;
    }

    const Stream &operator << (const uint8_t ch) const {
        writeFunc(ch);
        return *this;
    }

    const Stream &operator << (Decimal<uint8_t> v) const {
        if (v.value > 99) {
            writeFunc('0' + (v.value / 100));
            v.value %= 100;
        }
        if (v.value > 9) {
            writeFunc('0' + (v.value / 10));
            v.value %= 10;
        }
        writeFunc('0' + v.value);
        return *this;
    }
};



#endif /* STREAM_HPP_ */
