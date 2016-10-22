#ifndef STREAMS_WRITING_HPP_
#define STREAMS_WRITING_HPP_

#include "WritingN.hpp"
#include "HAL/Atmel/Registers.hpp"

namespace Streams {
namespace Impl {

inline void noop() {

}

using namespace HAL::Atmel::Registers;

template <typename fifo_t, void (*block)() = noop>
class BlockingWriteSemantics {
public:
    static inline constexpr bool isWriting(fifo_t &fifo) {
        return false;
    }

    static inline constexpr bool canWrite(fifo_t &fifo, uint8_t size) {
        return true;
    }

    static inline void write(fifo_t &fifo, uint8_t value) {
        if (SREG_I.isSet()) {
            uint16_t counter = 65000;
            while (fifo.isFull() && ((counter--) > 0)) block();
            fifo.uncheckedWrite(value);
            block();
        } else {
            if (fifo.hasSpace()) {
               fifo.uncheckedWrite(value);
            }
        }
    }

    static inline void start(fifo_t &fifo) {
        fifo.writeStart();
    }

    static inline void end(fifo_t &fifo, bool valid) {
        if (valid) {
            fifo.writeEnd();
        } else {
            fifo.writeAbort();
        }
    }
};

template <typename fifo_t>
class NonBlockingWriteSemantics {
public:
    static inline bool isWriting(fifo_t &fifo) {
        return fifo.isWriting();
    }

    static inline bool canWrite(fifo_t &fifo, uint8_t size) {
        return fifo.getSpace() >= size;
    }

    static inline void write(fifo_t &fifo, uint8_t value) {
        fifo.uncheckedWrite(value);
    }

    static inline void start(fifo_t &fifo) {
        fifo.writeStart();
    }

    static inline void end(fifo_t &fifo, bool valid) {
        if (valid) {
            fifo.writeEnd();
        } else {
            fifo.writeAbort();
        }
    }
};

// fifo_t will either be AbstractFifo(to be AbstractFifoReadable) or ChunkedFifo
template <typename sem, typename fifo_t, typename... types>
bool write(fifo_t &fifo, types... args) {
    const bool needStartEnd = !sem::isWriting(fifo);
    if (needStartEnd) {
        sem::start(fifo);
    }
    const bool success = writeN<sem>(fifo, args...);
    if (needStartEnd) {
        sem::end(fifo, success);
    }

    return success;
}

template <typename fifo_t, typename... types>
void writeOrBlock(fifo_t &fifo, types... args) {
    write<BlockingWriteSemantics<fifo_t>>(fifo, args...);
}

template <void (*block)(), typename fifo_t, typename... types>
void writeOrBlockWith(fifo_t &fifo, types... args) {
    write<BlockingWriteSemantics<fifo_t, block>>(fifo, args...);
}

template <typename fifo_t, typename... types>
bool writeIfSpace(fifo_t &fifo, types... args) {
    return write<NonBlockingWriteSemantics<fifo_t>>(fifo, args...);
}

}
}



#endif /* STREAMS_WRITING_HPP_ */
