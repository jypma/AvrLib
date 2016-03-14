#ifndef STREAMS_CHUNK_HPP_
#define STREAMS_CHUNK_HPP_

class ChunkedFifo;

namespace Streams {

class ChunkWithLength {
public:
    uint8_t * const length;
    ChunkedFifo * const fifo;
    constexpr ChunkWithLength(uint8_t *l, ChunkedFifo &f): length(l), fifo(&f) {}
};

}



#endif /* STREAMS_CHUNK_HPP_ */
