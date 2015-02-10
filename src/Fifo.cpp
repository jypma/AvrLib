#include "Fifo.hpp"

const Writer::VTable AbstractFifo::writerVTable = {
        &AbstractFifo::writeStart,
        &AbstractFifo::writeEnd,
        &AbstractFifo::write };

void AbstractFifo::writeStart(void *delegate) {
    ((AbstractFifo*)delegate)->markWrite();
}

void AbstractFifo::writeEnd(void *delegate) {
    ((AbstractFifo*)delegate)->commitWrite();
}

bool AbstractFifo::write(void *delegate, uint8_t b) {
    return ((AbstractFifo*)delegate)->append(b);
}

uint8_t AbstractFifo::markedOrWritePos() const {
    return (writeMark == NO_MARK) ? writePos : writeMark;
}

uint8_t AbstractFifo::markedOrReadPos() const {
    return (readMark == NO_MARK) ? readPos : readMark;
}

uint8_t AbstractFifo::getSize() const {
    AtomicScope _;
    return (markedOrWritePos() > markedOrReadPos()) ? markedOrWritePos() - markedOrReadPos() :
           (markedOrWritePos() < markedOrReadPos()) ? bufferSize - markedOrReadPos() + markedOrWritePos() :
           0;
}

bool AbstractFifo::append(uint8_t b) {
    AtomicScope _;

    if (hasSpace()) {
        buffer[writePos] = b;
        writePos = (writePos + 1) % bufferSize;
        return true;
    } else {
        return false;
    }
}

bool AbstractFifo::remove(uint8_t &b) {
    AtomicScope _;

    if (hasContent()) {
        b = buffer[readPos];
        readPos = (readPos + 1) % bufferSize;
        return true;
    } else {
        return false;
    }
}

void AbstractFifo::clear() {
    readPos = 0;
    writePos = 0;
    writeMark = NO_MARK;
    readMark = NO_MARK;
}

Writer AbstractFifo::out() {
    return Writer(&writerVTable, this);
}

bool AbstractFifo::isEmpty() const {
    AtomicScope _;
    return markedOrWritePos() == readPos;
}

bool AbstractFifo::hasContent() const {
    AtomicScope _;
    return markedOrWritePos() != readPos;
}

bool AbstractFifo::isFull() const {
    AtomicScope _;
    return writePos == (( markedOrReadPos() - 1 + bufferSize) % bufferSize);
}

uint8_t AbstractFifo::peek() const {
    AtomicScope _;
    if (hasContent()) {
        return buffer[readPos];
    } else {
        return 0;
    }
}

void AbstractFifo::resetWrite() {
    AtomicScope _;

    writePos = writeMark;
    writeMark = NO_MARK;
}

void AbstractFifo::resetRead() {
    AtomicScope _;

    readPos = readMark;
    readMark = NO_MARK;
}


