#ifndef STREAMS_STREAMINGDECL_HPP_
#define STREAMS_STREAMINGDECL_HPP_

#include "Strings.hpp"

namespace Streams {

constexpr auto endl = F("\r\n");

namespace Impl {

template <typename T>
class Reading {
    T *t() {
        return static_cast<T*>(this);
    }
public:
    /**
     * Attempts to read the given tokens or variables in sequence. Arguments can be any of the following:
     *
     * - Pointers to primitive variables, reading them as little endian (signed or unsigned, 8- 16- or 32-bit int).
     * - Constant strings as F("hello"), expecting that string in the content, failing otherwise
     *
     * Returns a ReadResult that contains the result of the read operation.
     */
    template <typename... types>
    Streams::ReadResult read(types... args);
};

template <typename T>
class Writing {
    T *t() {
        return static_cast<T*>(this);
    }
public:
    /**
     * Writes the given arguments to the fifo, blocking if and when there's not enough to space.
     *
     * This assumes some interrupt-driven routine regularly empties the fifo. That's the case
     * e.g. for Usarts.
     */
    template <typename... types>
    void writeOrBlock(types... args);

    /**
     * Writes the given arguments to the fifo, blocking if and when there's not enough to space.
     *
     * This assumes some interrupt-driven routine regularly empties the fifo. That's the case
     * e.g. for Usarts.
     *
     * When the implementation decides to block, it will continuously invoke the block() function.
     */
    template <void (*block)(), typename... types>
    void writeOrBlockWith(types... args);

    /**
     * Writes the given arguments to the fifo, or writes nothing if the arguments wouldn't fit in
     * the fifo.
     *
     * @return whether the value was written (true), or false if the Fifo was full.
     **/
    template <typename... types>
    bool writeIfSpace(types... args);
};

template <typename T>
class WritingDefaultIfSpace: public Writing<T> {
public:
    /**
     * Writes the given arguments to the fifo, or writes nothing if the arguments wouldn't fit in
     * the fifo. Equivalent to writeIfSpace.
     *
     * @return whether the value was written (true), or false if the Fifo was full.
     **/
    template <typename... types>
    bool write(types... args) {
        return Writing<T>::writeIfSpace(args...);
    }
};

template <typename T>
class Streaming: public Reading<T>, public Writing<T> {
};

template <typename T>
class StreamingDefaultWriteIfSpace: public Reading<T>, public WritingDefaultIfSpace<T> {
};

} // namespace Impl

template <typename T, typename fifo_t>
class ReadingDelegate: public ::Streams::Impl::Reading<T> {
    fifo_t *delegate;
public:
    ReadingDelegate(fifo_t *d): delegate(d) {}

    void readStart() { delegate->readStart(); }
    void readEnd() { delegate->readEnd(); }
    void readAbort() { delegate -> readAbort(); }
    void uncheckedRead(uint8_t &b) { delegate->uncheckedRead(b); }
    uint8_t getReadAvailable() const { return delegate->getReadAvailable(); }
    bool isReading() const { return delegate->isReading(); }
    bool hasContent() const { return delegate->hasContent(); }
};


}




#endif /* STREAMS_STREAMINGDECL_HPP_ */
