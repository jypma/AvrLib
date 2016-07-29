#ifndef STREAMS_WRITINGCHUNKEDFIFO_HPP_
#define STREAMS_WRITINGCHUNKEDFIFO_HPP_

#include "WritingBase.hpp"
#include "ChunkedFifoDecl.hpp"
#include "Logging.hpp"
#include "Nested.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t, typename src_fifo_t>
inline bool write1fifo(fifo_t &fifo, src_fifo_t &src) {
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


/**
 * Writes the remaining bytes from the open for reading ChunkedFifo into the target.
 * If the ChunkedFifo is not currently reading, the write is failed.
 *
 * Committing the fifo read has to be handled externally and in sync with the write itself succeeding.
 */
template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, ChunkedFifo &src) {
	return write1fifo<sem>(fifo, src);
}

/**
 * Writes the remaining bytes from the open for reading AbstractFifo into the target.
 * If the AbstractFifo is not currently reading, the write is failed.
 *
 * Committing the fifo read has to be handled externally and in sync with the write itself succeeding.
 */
template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, AbstractFifo &src) {
	return write1fifo<sem>(fifo, src);
}

/**
 * Writes the remaining bytes from the open for reading ReadingDelegate into the target.
 * If the ReadingDelegate is not currently reading, the write is failed.
 *
 * Committing the fifo read has to be handled externally and in sync with the write itself succeeding.
 */
template <typename sem, typename fifo_t, typename src_T, typename src_fifo_t>
bool write1(fifo_t &fifo, ReadingDelegate<src_T, src_fifo_t> &src) {
	return write1fifo<sem>(fifo, src);
}

/**
 * Writes the remaining bytes of the given nested read function into the target. This would be
 * invoked in a nested read block, e.g.
 *
 *     src.read(Nested([] (auto read) -> ReadResult { dest.write(read); }));
 *
 * This form allows for reading prefix data and handling that, before reading "the rest" into
 * a destination.
 */
template <typename sem, typename fifo_t, typename src_fifo_t>
bool write1(fifo_t &fifo, doread<src_fifo_t> n) {
	return write1fifo<sem>(fifo, *n.fifo);
}

}
}

#endif /* STREAMS_WRITINGCHUNKEDFIFO_HPP_ */
