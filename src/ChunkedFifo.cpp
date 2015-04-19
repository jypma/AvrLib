#include "ChunkedFifo.hpp"

const Writer::VTable ChunkedFifo::writerVTable = {
        &ChunkedFifo::writeStart,
        &ChunkedFifo::writeCommit,
        &ChunkedFifo::writeRollback,
        &ChunkedFifo::write };

void ChunkedFifo::writeStart(void *delegate) {
    ((ChunkedFifo*)delegate)->writeStart();
}

void ChunkedFifo::writeCommit(void *delegate) {
    ((ChunkedFifo*)delegate)->writeEnd();
}

void ChunkedFifo::writeRollback(void *delegate) {
    ((ChunkedFifo*)delegate)->writeAbort();
}

bool ChunkedFifo::write(void *delegate, uint8_t b) {
    return ((ChunkedFifo*)delegate)->write(b);
}

const Reader::VTable ChunkedFifo::readerVTable = {
        &ChunkedFifo::readStart,
        &ChunkedFifo::readCommit,
        &ChunkedFifo::readRollback,
        &ChunkedFifo::read,
        &ChunkedFifo::getAvailable };

void ChunkedFifo::readStart(void *delegate) {
    ((ChunkedFifo*)delegate)->readStart();
}

void ChunkedFifo::readCommit(void *delegate) {
    ((ChunkedFifo*)delegate)->readEnd();
}

void ChunkedFifo::readRollback(void *delegate) {
    ((ChunkedFifo*)delegate)->readAbort();
}

bool ChunkedFifo::read(void *delegate, uint8_t &b) {
    return ((ChunkedFifo*)delegate)->read(b);
}

uint8_t ChunkedFifo::getAvailable(void *delegate) {
    return ((ChunkedFifo*)delegate)->getReadAvailable();
}

void ChunkedFifo::clear() {
    AtomicScope _;
    data->clear();
}

bool ChunkedFifo::isFull() const {
    return data->isFull();
}

void ChunkedFifo::writeStart() {
    AtomicScope _;

    if (isWriting()) {
        data->resetWrite();
    }

    data->markWrite();
    writeValid = data->reserve(writeLengthPtr);
    if (writeValid) {
        *writeLengthPtr = 0;
    }
}

bool ChunkedFifo::write (uint8_t b) {
    AtomicScope _;

    if (writeValid && data->hasSpace()) {
        data->append(b);
        (*writeLengthPtr)++;
        return true;
    } else {
        writeValid = false;
        return false;
    }
}

void ChunkedFifo::writeEnd() {
    AtomicScope _;

    if (isWriting()) {
        if (writeValid) {
            data->commitWrite();
        } else {
            data->resetWrite();
        }
    }
}

void ChunkedFifo::readStart() {
    AtomicScope _;

    if (isReading()) {
        data->resetRead();
    }

    readValid = data->hasContent();
    if (readValid) {
        data->markRead();
        readValid = data->read(readLength);
    }
    if (!readValid) {
        readLength = 0;
    }
}

bool ChunkedFifo::read(uint8_t &ch) {
    AtomicScope _;

    if (readValid && readLength > 0) {
        readLength--;
        return data->read(ch);
    } else {
        readValid = false;
        readLength = 0;
        return false;
    }
}

void ChunkedFifo::readEnd() {
    AtomicScope _;

    if (isReading()) {
        if (readValid) {
            uint8_t dummy;
            while (readLength > 0) {
                data->read(dummy);
                readLength--;
            }
            data->commitRead();
        } else {
            data->resetRead();
        }

        readLength = 0;
    }
}

void ChunkedFifo::readAbort() {
    AtomicScope _;

    data->resetRead();
    readLength = 0;
}

Writer ChunkedFifo::out() {
    return Writer(&writerVTable, this);
}

Reader ChunkedFifo::in() {
    return Reader(&readerVTable, this);
}
