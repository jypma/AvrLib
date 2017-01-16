#ifndef STREAMS_READINGHEXADECIMAL_HPP_
#define STREAMS_READINGHEXADECIMAL_HPP_


#include "ReadingBase.hpp"
#include "Format.hpp"

namespace Streams {
namespace Impl {

static int8_t fromHex(uint8_t ch) {
    return ((ch >= '0') && (ch <= '9')) ? (ch - '0') :
           ((ch >= 'a') && (ch <= 'f')) ? (ch + 10 - 'a') :
           ((ch >= 'A') && (ch <= 'F')) ? (ch + 10 - 'A') :
           -1;
}

template <typename fifo_t>
ReadResult read1(fifo_t &fifo, Hexadecimal<uint8_t*> v) {
    if (fifo.getReadAvailable() < 2) {
        return ReadResult::Incomplete;
    }

    uint8_t hiChar, loChar;
    fifo.uncheckedRead(hiChar);
    int8_t hi = fromHex(hiChar);
    if (hi == -1) {
        return ReadResult::Invalid;
    }
    fifo.uncheckedRead(loChar);
    int8_t lo = fromHex(loChar);
    if (lo == -1) {
        return ReadResult::Invalid;
    }

    *v.value = (hi << 4) | lo;
    return ReadResult::Valid;
}

}
}


#endif /* STREAMS_READINGHEXADECIMAL_HPP_ */
