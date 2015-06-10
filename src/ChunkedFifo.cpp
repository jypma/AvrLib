#include "ChunkedFifo.hpp"

void ChunkedFifo::clear() {
    AtomicScope _;
    data->clear();
}

bool ChunkedFifo::isFull() const {
    return data->isFull();
}

void ChunkedFifo::writeStart() {
    AtomicScope _;

    if (writeInvocations == 0) {
        data->writeStart();
        writeValid = data->reserve(writeLengthPtr);
        if (writeValid) {
            *writeLengthPtr = 0;
        }
    }

    writeInvocations++;
}

bool ChunkedFifo::write (uint8_t b) {
    AtomicScope _;

    if (writeValid && data->hasSpace()) {
        data->write(b);
        (*writeLengthPtr)++;
        return true;
    } else {
        writeValid = false;
        return false;
    }
}

void ChunkedFifo::uncheckedWrite (uint8_t b) {
    data->uncheckedWrite(b);
    (*writeLengthPtr)++;
}

void ChunkedFifo::writeEnd() {
    AtomicScope _;

    if (isWriting()) {
        writeInvocations--;
        if (writeInvocations == 0) {
            if (writeValid) {
                data->writeEnd();
            } else {
                data->writeAbort();
            }
        }
    }
}


void ChunkedFifo::writeAbort() {
    AtomicScope _;

    if (isWriting()) {
        writeInvocations--;
        if (writeInvocations == 0) {
            data->writeAbort();
        }
    }
}

void ChunkedFifo::readStart() {
    AtomicScope _;

    if (readInvocations == 0) {
        readValid = data->hasContent();
        if (readValid) {
            data->readStart();
            readValid = data->read(readLength);
        }
        if (!readValid) {
            readLength = 0;
        }
    }

    readInvocations++;
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

void ChunkedFifo::uncheckedRead(uint8_t &ch) {
    readLength--;
    data->uncheckedRead(ch);
}

void ChunkedFifo::readEnd() {
    AtomicScope _;

    if (isReading()) {
        readInvocations--;
        if (readInvocations == 0) {
            if (readValid) {
                uint8_t dummy;
                while (readLength > 0) {
                    data->read(dummy);
                    readLength--;
                }
                data->readEnd();
            } else {
                data->readAbort();
            }

            readLength = 0;
        }
    }
}

void ChunkedFifo::readAbort() {
    AtomicScope _;

    if (isReading()) {
        readInvocations--;
        if (readInvocations == 0) {
            data->readAbort();
            readLength = 0;
        }
    }
}
