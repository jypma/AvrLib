#ifndef STREAMS_WRITINGCHUNKEDFIFO_HPP_
#define STREAMS_WRITINGCHUNKEDFIFO_HPP_

#include "WritingBase.hpp"
#include "ChunkedFifoDecl.hpp"
#include "Logging.hpp"

namespace Streams {
namespace Impl {

/**
 * Writes the remaining bytes from the open for reading ChunkedFifo into the target.
 * If the ChunkedFifo is not currently reading, the write is failed.
 *
 * Committing the fifo read has to be handled externally and in sync with the write itself succeeding.
 */
template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, ChunkedFifo &src) {
    if (!src.isReading()) {
        return false;
    }
    uint8_t count = src.getReadAvailable();
    if (sem::canWrite(fifo, count)) {
        for (; count > 0; count--) {
            uint8_t b;
            src.uncheckedRead(b);
            sem::write(fifo, b);
        }
        return true;
    } else {
        return false;
    }
}

}
}

#endif /* STREAMS_WRITINGCHUNKEDFIFO_HPP_ */
