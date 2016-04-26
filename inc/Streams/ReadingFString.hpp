#ifndef STREAMS_READINGFSTRING_HPP_
#define STREAMS_READINGFSTRING_HPP_

#include "ReadingBase.hpp"
#include "Strings.hpp"
#include "Logging.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t>
ReadResult verifyFromProgmem(fifo_t &fifo, uint8_t *addr, uint8_t length) {

    uint8_t available = fifo.getReadAvailable();
    if (available == 0) {
        return ReadResult::Incomplete;
    }
    uint8_t first;
    fifo.uncheckedRead(first);
    available--;

    const uint8_t expected = pgm_read_byte(addr);
    if (first != expected) {
        return ReadResult::Invalid;
    }

    for (uint8_t i = 1; i < length; i++) {
        if (available == 0) {
            return ReadResult::Partial;
        }
        uint8_t ch;
        fifo.uncheckedRead(ch);
        available--;

        addr++;
        const uint8_t expectedNext = pgm_read_byte(addr);
        if (ch != expectedNext) {
            return ReadResult::Invalid;
        }
    }

    return ReadResult::Valid;
}

template <typename fifo_t, uint8_t length>
ReadResult read1(fifo_t &fifo, StringInProgmem<length> *v) {
    return verifyFromProgmem(fifo, (uint8_t*) v, length);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, char expected) {
    uint8_t ch;
    fifo.uncheckedRead(ch);
    if (ch == expected) {
        return ReadResult::Valid;
    } else {
        return ReadResult::Invalid;
    }
}

}
}



#endif /* STREAMS_READINGFSTRING_HPP_ */
