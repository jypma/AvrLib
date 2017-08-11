#ifndef CHUNKEDFIFODECL_HPP_
#define CHUNKEDFIFODECL_HPP_

#include "FifoDecl.hpp"
#include "Streams/StreamingDecl.hpp"
#ifndef AVR
#include <iostream>
#endif

class AbstractChunkedFifo: public Streams::Impl::Reading<AbstractChunkedFifo> {
public:
    typedef Streams::Impl::ReadingDelegate<AbstractChunkedFifo> In;
#ifndef AVR
    friend inline ::std::ostream& operator<<(::std::ostream& os, AbstractChunkedFifo &that);
#endif

private:
    AbstractFifo * const data;

    volatile uint8_t *writeLengthPtr = nullptr;
    bool writeValid = false;

    uint8_t readLength = 0;
    bool readValid = false;

public:
    AbstractChunkedFifo(AbstractFifo &_data): data(&_data) {
    	getSize(); // In some instances, gcc -Os will "forget" to initialize data above, otherwise...
    }

    void clear();

    bool isFull() const;

    inline bool hasSpace() const {
        return !isFull();
    }

    inline bool hasContent() const {
        return data->hasContent();
    }

    inline uint8_t getCapacity() const {
    	return data->getCapacity();
    }

    inline uint8_t getSpace() const {
        return data->getSpace();
    }

    /**
     * Returns the number of bytes currently in the fifo, not counting any uncommitted reads or writes in progress,
     * but including any length markers.
     **/
    inline uint8_t getSize() const {
        return data->getSize();
    }

    inline uint8_t isEmpty() const {
        return data->isEmpty();
    }

    void writeStart();

    inline bool isWriting() const {
        return data->isWriting();
    }

    void uncheckedWrite(uint8_t b);

    void writeEnd();

    void writeAbort();

    void readStart();

    inline bool isReading() const {
        return data->isReading();
    }

    /** returns the number of bytes available in the current chunk, just after readStart() has been called. */
    inline uint8_t getReadAvailable() const {
        return readLength;
    }

    /** returns whether the current read, started by readStart, has no more bytes available */
    inline bool isReadComplete() const {
        return readLength == 0;
    }

    /** returns whether the current read, started by readStart, has more bytes available */
    inline bool hasReadAvailable() const {
        return readLength != 0;
    }

    uint8_t peek();

    void uncheckedRead(uint8_t &ch);

    void readEnd();

    void readAbort();

    inline In in() {
        return In(this);
    }
};


#ifndef AVR
inline ::std::ostream& operator<<(::std::ostream& os, AbstractChunkedFifo &that) {
    if (that.isEmpty()) {
        os << "{}";
        return os;
    }
    that.data->readStart();
    uint8_t length;
    const char *separator = "";
    while (that.data->getReadAvailable()) {
        that.data->read(&length);
        os << separator << int(length) << ": [";
        while (length && that.data->getReadAvailable()) {
            uint8_t b;
            that.data->read(&b);
            os << " " << int(b);
            length--;
        }
        os << "]";
        separator = ", ";
    }
    that.data->readAbort();
    return os;
}
#endif

class ChunkedFifo: public AbstractChunkedFifo, public Streams::Impl::WritingDefaultIfSpace<ChunkedFifo> {
public:
    using AbstractChunkedFifo::AbstractChunkedFifo;
};

template <typename callback_t, typename target_t>
class ChunkedFifoCB: public AbstractChunkedFifo, public Streams::Impl::WritingDefaultIfSpace<ChunkedFifoCB<callback_t, target_t>> {
    typedef ChunkedFifoCB<callback_t, target_t> This;

    target_t *target;
public:
    ChunkedFifoCB(AbstractFifo &_data, target_t &_target): AbstractChunkedFifo(_data), target(&_target) {}

    void writeEnd() {
        AbstractChunkedFifo::writeEnd();
        if (!isWriting()) {
            callback_t::onWriteEnd(*target);
        }
    }
};


#endif /* CHUNKEDFIFODECL_HPP_ */
