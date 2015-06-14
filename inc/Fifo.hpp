/*
 * Fifo.hpp
 *
 *  Created on: Jan 5, 2015
 *      Author: jan
 */

#ifndef FIFO_HPP_
#define FIFO_HPP_

#include "AtomicScope.hpp"
#include "Streams/Writer.hpp"
#include "Streams/Reader.hpp"

/**
 * A FIFO queue of bytes, with a maximum size of 254.
 */
class AbstractFifo {
    constexpr static uint8_t NO_MARK = 255;

    volatile uint8_t * const buffer;
    const uint8_t bufferSize;
    volatile uint8_t readPos = 0;
    volatile uint8_t writePos = 0;
    uint8_t writeMark = NO_MARK;
    uint8_t readMark = NO_MARK;
    uint8_t readMarkInvocations = 0;
    uint8_t writeMarkInvocations = 0;

public:
    inline bool isWriting() const {
        return writeMarkInvocations > 0;
    }
    inline bool isReading() const {
        return readMarkInvocations > 0;
    }

private:
    inline uint8_t markedOrWritePos() const {
        return isWriting() ? writeMark : writePos;
    }
    inline uint8_t markedOrReadPos() const {
        return isReading() ? readMark : readPos;
    }
    inline bool _isFull() const {
        uint8_t lastWritePos = markedOrReadPos();
        if (lastWritePos == 0) {
            lastWritePos = bufferSize - 1;
        } else {
            lastWritePos--;
        }
        return writePos == lastWritePos;
    }
    inline bool _hasSpace() const {
        uint8_t lastWritePos = markedOrReadPos();
        if (lastWritePos == 0) {
            lastWritePos = bufferSize - 1;
        } else {
            lastWritePos--;
        }
        return writePos != lastWritePos;
    }

public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _bufferSize): buffer(_buffer), bufferSize(_bufferSize) {}

    bool isEmpty() const;

    bool hasContent() const;

    bool isFull() const;

    inline bool hasSpace() const {
        return !isFull();
    }

    uint8_t getSize() const;

    inline uint8_t getReadAvailable() const {
        return getSize();
    }

    uint8_t getSpace() const;

    inline uint8_t getCapacity() const {
        return bufferSize - 1;
    }

    uint8_t peek() const;


    void writeStart();

    void writeEnd();

    void writeAbort();

    void readStart();

    void readEnd();

    void readAbort();

    /** Returns whether the value was appended (true), or false if the Fifo was full. */
    bool write(uint8_t b);

    /**
     * Appends the given element, assuming there is space for it. May only be called after having
     * previously confirmed enough space by checking getAvailable(), and calling markWrite(). That
     * also makes sure that no interrupt code can get confused while unchecked appends are being made.
     */
    void uncheckedWrite(uint8_t b);

    /** Valid to call between markWrite() and commitWrite(), to reserve a spot in the queue,
     * with the actual value being set later. The caller must NOT write to [ptr] after
     * markWrite() has been called.
     *
     * Returns whether the ptr was reserved (true), or false if the Fifo was full,
     * or no write mark was set.
     */
    bool reserve(volatile uint8_t * &ptr);

    /** Returns whether a value was removed and updated in b (true), or false if the Fifo was empty */
    bool read(uint8_t &b);

    /**
     * Reads a value from the fifo, assuming that previously a check to getSize() was made,
     * and nothing else was read in the meantime.
     */
    void uncheckedRead(uint8_t &b);

    void clear();

    //Writer out();

    //Reader in();

    inline Streams::Writer<AbstractFifo> out() {
        return Streams::Writer<AbstractFifo>(*this);
    }

    inline Streams::Reader<AbstractFifo> in() {
        return Streams::Reader<AbstractFifo>(*this);
    }

    /** Only for use in interrupts. Inlined, and does not disable interrupt flag. */
    inline bool fastread(uint8_t &b) {
        const bool avail = markedOrWritePos() != readPos;
        if (avail) {
            b = buffer[readPos];
            readPos++;
            if (readPos >= bufferSize) {
                readPos -= bufferSize;
            }
        }
        return avail;
    }

    /** Only for use in interrupts. Inlined, and does not disable interrupt flag. */
    inline void fastwrite(uint8_t b) {
        if (hasSpace()) {
            buffer[writePos] = b;
            writePos++;
            if (writePos >= bufferSize) {
                writePos -= bufferSize;
            }
        }
    }
};

/**
 * Statically allocated FIFO
 */
template<uint8_t Capacity>
class Fifo: public AbstractFifo {
    uint8_t buffer[Capacity + 1];
public:
    Fifo(): AbstractFifo(buffer, Capacity + 1) {}
};
#endif /* FIFO_HPP_ */
