/*
 * Reader.hpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jan
 */

#ifndef READER_HPP_
#define READER_HPP_

#include <stdint.h>

namespace Streams {

template <typename fifo_t>
class Reader {
    fifo_t *fifo;
    bool valid = true;

    template <typename T>
    inline void readLiteral(T &value) {
        if (valid && (fifo->getReadAvailable() >= sizeof(T))) {
            for (uint8_t i = 0; i < sizeof(T); i++) {
                fifo->uncheckedRead( ((uint8_t*)(&value))[i] );
            }
        } else {
            valid = false;
        }
    }

public:
    inline Reader(fifo_t &_fifo): fifo(&_fifo) {
        fifo->readStart();
    }
    inline ~Reader() {
        if (valid) {
            fifo->readEnd();
        } else {
            fifo->readAbort();
        }
    }

    inline operator bool() const {
        return valid;
    }

    /** Returns the amount of bytes available to be read. */
    inline uint8_t getReadAvailable() const {
        return fifo->getReadAvailable();
    }

    /** Reads a struct that has a configured protocol by declaring a typedef of Streams::Format<...> Proto. */
    template <typename T, typename check = typename T::Proto>
    inline Reader & operator >> (T &t) {
        typedef typename T::Proto P;

        if (valid && fifo->getReadAvailable() >= P::minimumSize) {
            P::readFields(*fifo, t);
        } else {
            valid = false;
        }

        return *this;
    }

    /** Reads a single byte */
    inline Reader & operator >> (uint8_t &b) {
        if (valid && fifo->getReadAvailable() >= 1) {
            fifo->uncheckedRead(b);
        } else {
            valid = false;
        }

        return *this;
    }

    /** Reads a literal pointer (the actual address), LSB first (little endian) */
    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline Reader &operator >> (T &t) {
        readLiteral(t);
        return *this;
    }

};

}



#endif /* READER_HPP_ */
