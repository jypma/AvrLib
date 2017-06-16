#ifndef STREAMS_STREAMINGDECL_HPP_
#define STREAMS_STREAMINGDECL_HPP_

#include "Strings.hpp"

namespace Streams {

constexpr auto endl = F("\r\n");

namespace Impl {

template <typename fifo_t>
class ReadingDelegate;

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

template <typename sem, typename fifo_t, typename src_fifo_t>
bool write1(fifo_t &fifo, ReadingDelegate<src_fifo_t> src);

template <typename fifo_t>
class ReadingDelegate {
    fifo_t * const delegate;
public:
    constexpr ReadingDelegate(fifo_t *d): delegate(d) {}
    constexpr ReadingDelegate(const ReadingDelegate &r): delegate(r.delegate) {}

    template <typename... types>
    Streams::ReadResult read(types... args) {
        return delegate->read(args...);
    }

    template <typename sem, typename f_t, typename src_fifo_t>
    friend bool ::Streams::Impl::write1(f_t &fifo, ReadingDelegate<src_fifo_t> src);

    inline void clear() { delegate->clear(); }
    inline uint8_t getSize() const { return delegate->getSize(); }
    inline bool isEmpty() const { return delegate->isEmpty(); }
    inline void readStart() { delegate->readStart(); }
    inline void readEnd() { delegate->readEnd(); }
    inline void readAbort() { delegate -> readAbort(); }
    inline void uncheckedRead(uint8_t &b) { delegate->uncheckedRead(b); }
    inline uint8_t getReadAvailable() const { return delegate->getReadAvailable(); }
    inline bool isReading() const { return delegate->isReading(); }
    inline bool hasContent() const { return delegate->hasContent(); }
    inline uint8_t peek() const { return delegate->peek(); }
};

} // namespace Impl

}




#endif /* STREAMS_STREAMINGDECL_HPP_ */
