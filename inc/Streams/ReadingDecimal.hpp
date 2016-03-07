#ifndef STREAMS_READINGDECIMAL_HPP_
#define STREAMS_READINGDECIMAL_HPP_

#include "ReadingBase.hpp"
#include "Format.hpp"

namespace Streams {
namespace Impl {

inline bool isDigit(uint8_t digit) {
    return digit >= '0' && digit <= '9';
}

template <typename fifo_t>
ReadResult read1(fifo_t &fifo, Decimal<uint8_t*> v) {
    if (fifo.getReadAvailable() < 1) {
        return ReadResult::Incomplete;
    }

    *(v.value) = 0;
    uint8_t digit;
    fifo.uncheckedRead(digit); // digit 0
    if (isDigit(digit)) {
        if (digit != '0') {
            *(v.value) = digit - '0';
            uint8_t available = fifo.getReadAvailable();
            if (available > 0) {
                if (isDigit(fifo.peek())) {
                    fifo.uncheckedRead(digit); // digit 1
                    *(v.value) = *(v.value) * 10 + (digit - '0');
                    available--;
                    if (available > 0) {
                        if (isDigit(fifo.peek())) {
                            fifo.uncheckedRead(digit); // digit 2
                            if ((*(v.value) <= 24) || (*(v.value) == 25 && digit <= '5')) {
                                *(v.value) = *(v.value) * 10 + (digit - '0');
                            } else {
                                // Overflow for uint8_t, that's not OK.
                                return ReadResult::Invalid;
                            }
                        } else {
                            // Non-digit as third char, that's OK
                        }
                    } else {
                        // No third char
                    }
                } else {
                    // Non-digit as second char, that's OK
                }
            } else {
                // Only one digit, that's OK
            }
        } else {
            // First digit was '0', so length was zero.
        }
    } else {
        // Expecting a digit as first char, didn't get one
        return ReadResult::Invalid;
    }

    return ReadResult::Valid;
}

}
}



#endif /* STREAMS_READINGDECIMAL_HPP_ */
