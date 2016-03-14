#include "ChunkedFifo.hpp"

void AbstractChunkedFifo::clear() {
    AtomicScope _;
    data->clear();
}

bool AbstractChunkedFifo::isFull() const {
    return data->isFull();
}

void AbstractChunkedFifo::writeStart() {
    AtomicScope _;

    if (!isWriting()) {
        data->writeStart();
        writeValid = data->reserve(writeLengthPtr);
        if (writeValid) {
            *writeLengthPtr = 0;
        }
    }
}

void AbstractChunkedFifo::uncheckedWrite (uint8_t b) {
    data->uncheckedWrite(b);
    (*writeLengthPtr)++;
}

void AbstractChunkedFifo::writeEnd() {
    AtomicScope _;

    if (isWriting()) {
        if (writeValid) {
            data->writeEnd();
        } else {
            data->writeAbort();
        }
    }
}


void AbstractChunkedFifo::writeAbort() {
    data->writeAbort();
}

void AbstractChunkedFifo::readStart() {
    AtomicScope _;

    if (!isReading()) {
        readValid = data->hasContent();
        if (readValid) {
            data->readStart();
            readValid = data->read(&readLength);
        }
        if (!readValid) {
            readLength = 0;
        }
    }
}

uint8_t AbstractChunkedFifo::peek() {
    AtomicScope _;

    if (readValid && readLength > 0) {
        return data->peek();
    } else {
        return 0;
    }
}

void AbstractChunkedFifo::uncheckedRead(uint8_t &ch) {
    readLength--;
    data->uncheckedRead(ch);
}

void AbstractChunkedFifo::readEnd() {
    AtomicScope _;

    if (isReading()) {
        if (readValid) {
            uint8_t dummy;
            while (readLength > 0) {
                data->read(&dummy);
                readLength--;
            }
            data->readEnd();
        } else {
            data->readAbort();
        }

        readLength = 0;
    }
}

void AbstractChunkedFifo::readAbort() {
    AtomicScope _;

    if (isReading()) {
        data->readAbort();
        readLength = 0;
    }
}
