#ifndef STREAMS_READINGFSTRING_HPP_
#define STREAMS_READINGFSTRING_HPP_

#include "ReadingBase.hpp"
#include "Strings.hpp"
#include "Logging.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t>
ReadResult verifyFromProgmem(fifo_t &fifo, uint8_t *addr, uint8_t length) {
    typedef Logging::Log<Loggers::Streams> log;

    uint8_t available = fifo.getReadAvailable();
    if (available == 0) {
        log::debug("Nothing available");
        return ReadResult::Incomplete;
    }
    uint8_t first;
    fifo.uncheckedRead(first);
    available--;

    const uint8_t expected = pgm_read_byte(addr);
    if (first != expected) {
        log::debug("Expecting %d(%c) at 0, got %d, marking invalid.", expected, expected, first);
        return ReadResult::Invalid;
    }

    for (uint8_t i = 1; i < length; i++) {
        if (available == 0) {
            log::debug("No more available, marking partial");
            return ReadResult::Partial;
        }
        uint8_t ch;
        fifo.uncheckedRead(ch);
        available--;

        addr++;
        const uint8_t expectedNext = pgm_read_byte(addr);
        if (ch != expectedNext) {
            log::debug("Expected %d(%c) at %d, got %d, marking invalid.", expectedNext, expectedNext, i, ch);
            return ReadResult::Invalid;
        }
    }

    return ReadResult::Valid;
}

template <typename fifo_t, uint8_t length>
ReadResult read1(fifo_t &fifo, StringInProgmem<length> *v) {
    return verifyFromProgmem(fifo, (uint8_t*) v, length);
}

}
}



#endif /* STREAMS_READINGFSTRING_HPP_ */
