#ifndef STREAMS_READING_HPP_
#define STREAMS_READING_HPP_

#include "Logging.hpp"
#include "AtomicScope.hpp"
#include "ReadResult.hpp"
#include "ReadingN.hpp"

namespace Streams {

namespace Impl {

// fifo_t will either be AbstractFifo(to be AbstractFifoReadable) or ChunkedFifo

template <typename fifo_t, typename... types>
ReadResult read(fifo_t &fifo, types... args) {
    AtomicScope _;

    const bool needStartEnd = !fifo.isReading();
    if (needStartEnd) {
        fifo.readStart();
    }
    const ReadResult result = readN(fifo, ReadResult::Valid, args...);
    if (needStartEnd) {
        if (result == ReadResult::Valid) {
            fifo.readEnd();
        } else {
            fifo.readAbort();
        }
    }
    return result;
}

} // namespace Impl
} // namespace Streams

#endif /* STREAMS_READING_HPP_ */
