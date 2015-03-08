/*
 * Fifo.hpp
 *
 *  Created on: Jan 5, 2015
 *      Author: jan
 */

#ifndef FIFO_HPP_
#define FIFO_HPP_

#include "AtomicScope.hpp"
#include "Writer.hpp"
#include "Reader.hpp"

/**
 * A FIFO queue of bytes, with a maximum size of 254.
 */
class AbstractFifo {
    constexpr static uint8_t NO_MARK = 255;

    const static Writer::VTable writerVTable;
    static void writeStart(void *delegate);
    static void writeEnd(void *delegate);
    static bool write(void *delegate, uint8_t b);

    const static Reader::VTable readerVTable;
    static void readStart(void *delegate);
    static void readEnd(void *delegate);
    static bool read(void *delegate, uint8_t &target);
    static uint8_t getRemaining(void *delegate);

    volatile uint8_t * const buffer;
    const uint8_t bufferSize;
    volatile uint8_t readPos = 0;
    volatile uint8_t writePos = 0;
    uint8_t writeMark = NO_MARK;
    uint8_t readMark = NO_MARK;

    uint8_t markedOrWritePos() const;
    uint8_t markedOrReadPos() const;
public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _bufferSize): buffer(_buffer), bufferSize(_bufferSize) {}

    bool isEmpty() const;

    bool hasContent() const;

    bool isFull() const;

    inline bool hasSpace() const {
        return !isFull();
    }

    uint8_t getSize() const;

    inline uint8_t getCapacity() const {
        return bufferSize - 1;
    }

    uint8_t peek() const;

    inline void markWrite() {
        writeMark = writePos;
    }

    inline bool isWriteMarked() {
        return writeMark != NO_MARK;
    }

    inline void commitWrite() {
        writeMark = NO_MARK;
    }

    void resetWrite();

    inline void markRead() {
        readMark = readPos;
    }

    inline bool isReadMarked() {
        return readMark != NO_MARK;
    }

    inline void commitRead() {
        readMark = NO_MARK;
    }

    void resetRead();

    /** Returns whether the value was appended (true), or false if the Fifo was full. */
    bool append(uint8_t b);

    /** Returns whether a value was removed and updated in b (true), or false if the Fifo was empty */
    bool remove(uint8_t &b);

    void clear();

    Writer out();

    Reader in();
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
