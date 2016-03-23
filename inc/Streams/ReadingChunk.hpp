#ifndef STREAMS_READINGCHUNK_HPP_
#define STREAMS_READINGCHUNK_HPP_

#include "ChunkedFifoDecl.hpp"
#include "ReadingBase.hpp"
#include "Streams/Chunk.hpp"
#include "Logging.hpp"

namespace Streams {
namespace Impl {


/**
 * Reads a chunk of bytes into the open for writing ChunkedFifo indicated by [c]. The length member of [c] must occur
 * as a target argument earlier on in the same call to read(), since we have to know the length before
 * reading the chunk.
 *
 * If the ChunkedFifo is not currently writing, the whole chunk is dropped.
 *
 * If the ChunkedFifo does not have enough space, the whole chunk is dropped. It has to, since otherwise a deadlock can
 * occur between the source not completing and the target never getting enough space.
 */
template <typename fifo_t>
ReadResult read1(fifo_t &fifo, ChunkWithLength c) {

    uint8_t count = *c.length;
    if (fifo.getReadAvailable() >= count) {
        if (c.fifo->isWriting() && c.fifo->getSpace() >= count) {
            for (; count > 0; count--) {
                uint8_t ch;
                fifo.uncheckedRead(ch);
                c.fifo->uncheckedWrite(ch);
            }
        } else {
            for (; count > 0; count--) {
                uint8_t ch;
                fifo.uncheckedRead(ch);
            }
        }
        return ReadResult::Valid;
    } else {
        return ReadResult::Incomplete;
    }
}

}
}




#endif /* STREAMS_READINGCHUNK_HPP_ */
