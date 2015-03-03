#include "ChunkedFifo.hpp"

const Writer::VTable ChunkedFifo::writerVTable = {
        &ChunkedFifo::writeStart,
        &ChunkedFifo::writeEnd,
        &ChunkedFifo::write };

void ChunkedFifo::writeStart(void *delegate) {
    ((ChunkedFifo*)delegate)->writeStart();
}

void ChunkedFifo::writeEnd(void *delegate) {
    ((ChunkedFifo*)delegate)->writeEnd();
}

bool ChunkedFifo::write(void *delegate, uint8_t b) {
    return ((ChunkedFifo*)delegate)->write(b);
}

const Reader::VTable ChunkedFifo::readerVTable = {
        &ChunkedFifo::readStart,
        &ChunkedFifo::readEnd,
        &ChunkedFifo::read,
        &ChunkedFifo::getAvailable };

void ChunkedFifo::readStart(void *delegate) {
    ((ChunkedFifo*)delegate)->readStart();
}

void ChunkedFifo::readEnd(void *delegate) {
    ((ChunkedFifo*)delegate)->readEnd();
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
    lengths->clear();
}

bool ChunkedFifo::isFull() const {
    return data->isFull() || lengths->isFull();
}

void ChunkedFifo::writeStart() {
    AtomicScope _;

    if (isWriting()) {
        data->resetWrite();
    }

    data->markWrite();
    writeLength = 0;
    writeValid = true;
}

bool ChunkedFifo::write (uint8_t b) {
    AtomicScope _;

    if (writeValid && data->hasSpace()) {
        data->append(b);
        writeLength++;
        return true;
    } else {
        writeValid = false;
        return false;
    }
}

void ChunkedFifo::writeEnd() {
    AtomicScope _;

    if (isWriting()) {
        if (writeValid && lengths->hasSpace()) {
            data->commitWrite();
            lengths->append(writeLength);
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

    readValid = lengths->hasContent();
    if (readValid) {
        readLength = lengths->peek();
        data->markRead();
    } else {
        readLength = 0;
    }
}

bool ChunkedFifo::read(uint8_t &ch) {
    AtomicScope _;

    if (readValid && readLength > 0) {
        readLength--;
        return data->remove(ch);
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
                data->remove(dummy);
                readLength--;
            }
            lengths->remove(dummy);
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
