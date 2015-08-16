/*
 * Reader.hpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jan
 */

#ifndef READER_HPP_
#define READER_HPP_

#include <stdint.h>
#include <gcc_type_traits.h>

namespace Streams {

enum class ReaderState { Valid, Invalid, Incomplete, Partial };

template <typename fifo_t>
class Reader {
    fifo_t *fifo;
    ReaderState state = ReaderState::Valid;

    template <typename T>
    inline void readLiteral(T &value) {
        if (isValid() && (fifo->getReadAvailable() >= sizeof(T))) {
            for (uint8_t i = 0; i < sizeof(T); i++) {
                fifo->uncheckedRead( ((uint8_t*)(&value))[i] );
            }
        } else {
            markIncomplete();
        }
    }

public:
    inline Reader(fifo_t &_fifo): fifo(&_fifo) {
        fifo->readStart();
    }
    inline ~Reader() {
        if (isValid()) {
            fifo->readEnd();
        } else {
            fifo->readAbort();
        }
    }

    void markIncomplete() {
        if (state != ReaderState::Invalid) {
            state = ReaderState::Incomplete;
        }
    }

    void markPartial() {
        if (state != ReaderState::Invalid) {
            state = ReaderState::Partial;
        }
    }

    void markInvalid() {
        state = ReaderState::Invalid;
    }

    inline bool isValid() const {
        return state == ReaderState::Valid;
    }

    inline operator bool() const {
        return isValid();
    }

    inline operator ReaderState() const {
        return state;
    }

    inline void uncheckedRead(uint8_t &b) {
        if (isValid()) { // FIXME check if this might be too slow, then we can group sets of Scalar<> inside a Format<> in a direct way.
            fifo->uncheckedRead(b);
        }
    }

    inline uint8_t peek() const {
        return fifo->peek();
    }

    /** Returns the amount of bytes available to be read. */
    inline uint8_t getReadAvailable() const {
        return fifo->getReadAvailable();
    }

    template <typename Proto, typename T>
    inline Reader &readAs(T &t) {
        if (isValid()) {
            Proto::read(*this, t);
        }
        return *this;
    }

    template <typename Proto>
    inline Reader &expect() {
        if (isValid()) {
            Proto::read(*this);
        }
        return *this;
    }

    /** Reads a struct that has a configured protocol by declaring a typedef of Streams::Format<...> Proto. */
    template <typename T, typename check = typename T::Proto>
    inline Reader & operator >> (T &t) {
        readAs<typename T::Proto, T>(t);
        return *this;
    }

    /** Reads a single byte */
    inline Reader & operator >> (uint8_t &b) {
        if (isValid() && fifo->getReadAvailable() >= 1) {
            fifo->uncheckedRead(b);
        } else {
            markIncomplete();
        }

        return *this;
    }

    /** Reads a literal pointer (the actual address), LSB first (little endian) */
    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline Reader &operator >> (T &t) {
        readLiteral(t);
        return *this;
    }

    /*
    // FIXME rewrite as scan<Token<...>>
    Reader & operator >> (const char *token) {
        if (token == nullptr) {
            return *this;
        }

        bool foundToken = false;
        while (isValid() && *token != '\0') {
            if (fifo->getReadAvailable() >= 1) {
                uint8_t incoming;
                fifo->uncheckedRead(incoming);
                if (foundToken) {
                    if (incoming == *token) {
                        token++;
                    } else {
                        markInvalid();
                    }
                } else {
                    if (incoming == *token) {
                        foundToken = true;
                        token++;
                    }
                }
            } else {
                markIncomplete();
            }
        }

        return *this;
    }
    */
};

}



#endif /* READER_HPP_ */
