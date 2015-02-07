/*
 * Fifo.hpp
 *
 *  Created on: Jan 5, 2015
 *      Author: jan
 */

#ifndef FIFO_HPP_
#define FIFO_HPP_

#include "Interrupt.hpp"
#include "Writer.hpp"

/**
 * A FIFO queue of bytes, with a maximum size of 254.
 */
class AbstractFifo {
    constexpr static uint8_t NO_MARK = 255;

    const static Writer::VTable writerVTable;
    static void writeStart(void *delegate);
    static void writeEnd(void *delegate);
    static bool write(void *delegate, uint8_t b);

    volatile uint8_t * const buffer;
    const uint8_t capacity;
    volatile uint8_t head = 0;
    volatile uint8_t end = 0;
    uint8_t writeMark = NO_MARK;
    uint8_t readMark = NO_MARK;
    bool endAfterReadReset = false;

    uint8_t markedEnd() const {
        return (endAfterReadReset) ? 0 : (writeMark == NO_MARK) ? end : writeMark;
    }
    uint8_t markedHead() const {
        return (readMark == NO_MARK) ? head : readMark;
    }
public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _capacity): buffer(_buffer), capacity(_capacity) {}

    bool isEmpty() const {
        AtomicScope _;
        return markedEnd() == markedHead();
    }

    bool hasContent() const {
        AtomicScope _;
        return markedEnd() != markedHead();
    }

    bool isFull() const {
        AtomicScope _;
        return (head == 0) ? end == capacity : end == head - 1;
    }

    bool hasSpace() const {
        AtomicScope _;
        return (head == 0) ? end != capacity : end != head - 1;
    }

    uint8_t getSize() const;

    uint8_t getCapacity() const {
        return capacity;
    }

    uint8_t peek() const {
        if (hasContent()) {
            return buffer[head];
        } else {
            return 0;
        }
    }

    void markWrite() {
        writeMark = end;
    }

    bool isWriteMarked() {
        return writeMark != NO_MARK;
    }

    void commitWrite() {
        writeMark = NO_MARK;
    }

    void resetWrite() {
        AtomicScope _;

        end = writeMark;
        writeMark = NO_MARK;
    }

    void markRead() {
        AtomicScope _;

        readMark = head;
    }

    bool isReadMarked() {
        return readMark != NO_MARK;
    }

    void commitRead() {
        readMark = NO_MARK;
        if (endAfterReadReset) {
            end = 0;
        }
        endAfterReadReset = false;
    }

    void resetRead() {
        AtomicScope _;

        head = readMark;
        readMark = NO_MARK;
        endAfterReadReset = false;
    }

    /** Returns whether the value was appended (true), or false if the Fifo was full. */
    bool append(uint8_t b);

    /** Returns whether a value was removed and updated in b (true), or false if the Fifo was empty */
    bool remove(uint8_t &b);

    void clear();

    Writer out();
};

/**
 * Statically allocated FIFO
 */
template<uint8_t Capacity>
class Fifo: public AbstractFifo {
    uint8_t buffer[Capacity];
public:
    Fifo(): AbstractFifo(buffer, Capacity) {}
};
#endif /* FIFO_HPP_ */
