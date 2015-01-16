/*
 * Fifo.hpp
 *
 *  Created on: Jan 5, 2015
 *      Author: jan
 */

#ifndef FIFO_HPP_
#define FIFO_HPP_

#include "Interrupt.hpp"

/**
 * A FIFO queue of bytes, with a maximum size of 255.
 */
class AbstractFifo {
    volatile uint8_t * const buffer;
    const uint8_t capacity;
    volatile uint8_t head = 0;
    volatile uint8_t end = 0;
public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _capacity): buffer(_buffer), capacity(_capacity) {}

    bool isEmpty() const {
        AtomicScope _;
        return end == head;
    }

    bool hasContent() const {
        AtomicScope _;
        return end != head;
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

    void append(uint8_t b);

    uint8_t remove();
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
