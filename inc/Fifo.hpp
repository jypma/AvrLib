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
    uint8_t * const buffer;
    uint8_t const capacity;
    uint8_t head = 0;
    uint8_t end = 0;
public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _capacity): buffer(_buffer), capacity(_capacity) {}

    bool isEmpty() const {
        ScopedNoInterrupts cli;
        return end == head;
    }

    bool hasContent() const {
        ScopedNoInterrupts cli;
        return end != head;
    }

    bool isFull() const {
        ScopedNoInterrupts cli;
        return (head == 0) ? end == capacity : end == head - 1;
    }

    bool hasSpace() const {
        ScopedNoInterrupts cli;
        return (head == 0) ? end != capacity : end != head - 1;
    }

    uint8_t getSize() const {
        ScopedNoInterrupts cli;
        return (end > head) ? end - head :
               (end < head) ? capacity - head + end :
               0;
    }

    uint8_t getCapacity() const {
        return capacity;
    }

    void append(uint8_t b) {
        ScopedNoInterrupts cli;

        if (hasSpace()) {
            if (end < capacity) {
                buffer[end] = b;
                end++;
            } else {
                buffer[0] = b;
                end = 1;
            }
        }
    }

    uint8_t remove() {
        ScopedNoInterrupts cli;

        if (hasContent()) {
            uint8_t result = buffer[head];
            if (head < capacity - 1) {
                head++;
            } else {
                head = 0;
                if (end == capacity) {
                    end = 0;
                }
            }
            return result;
        } else {
            return 0;
        }
    }
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
