#include "Fifo.hpp"

uint8_t AbstractFifo::getSize() const {
    AtomicScope _;
    return (end > head) ? end - head :
           (end < head) ? capacity - head + end :
           0;
}

void AbstractFifo::append(uint8_t b) {
    AtomicScope _;

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

uint8_t AbstractFifo::remove() {
    AtomicScope _;

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
