/*
 * Stream.hpp
 *
 *  Created on: Jan 15, 2015
 *      Author: jan
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include <avr/common.h>

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
};



#endif /* STREAM_HPP_ */
