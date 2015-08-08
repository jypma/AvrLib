/*
 * ChunkedFifo.hpp
 *
 *  Created on: Jan 21, 2015
 *      Author: jan
 */

#ifndef CHUNKEDFIFO_HPP_
#define CHUNKEDFIFO_HPP_

#include "Fifo.hpp"

class ChunkedFifo {
    AbstractFifo *data;

    volatile uint8_t *writeLengthPtr = nullptr;
    bool writeValid = false;

    uint8_t readLength = 0;
    bool readValid = false;

    uint8_t readInvocations = 0;
    uint8_t writeInvocations = 0;

public:
    ChunkedFifo(AbstractFifo *_data): data(_data) {}

    void clear();

    bool isFull() const;

    inline bool hasContent() const {
        return data->hasContent();
    }

    inline uint8_t getSpace() const {
        return data->getSpace();
    }

    inline uint8_t getSize() const {
        return data->getSize();
    }

    inline uint8_t isEmpty() const {
        return data->isEmpty();
    }

    void writeStart();

    inline bool isWriting() const {
        return writeInvocations > 0;
    }

    bool write (uint8_t b);

    void uncheckedWrite(uint8_t b);

    void writeEnd();

    void writeAbort();

    void readStart();

    inline bool isReading() const {
        return readInvocations > 0;
    }

    /** returns the number of bytes available in the current chunk, just after readStart() has been called. */
    inline uint8_t getReadAvailable() const {
        return readLength;
    }

    /** returns whether the current read, started by readStart, has no more bytes available */
    inline bool isReadComplete() const {
        return readLength == 0;
    }

    /** returns whether the current read, started by readStart, has more bytes available */
    inline bool hasReadAvailable() const {
        return readLength != 0;
    }

    bool read(uint8_t &ch);

    void uncheckedRead(uint8_t &ch);

    void readEnd();

    void readAbort();

    inline Streams::Writer<ChunkedFifo> out() {
        return Streams::Writer<ChunkedFifo>(*this);
    }

    inline Streams::Reader<ChunkedFifo> in() {
        return Streams::Reader<ChunkedFifo>(*this);
    }
};

#endif /* CHUNKEDFIFO_HPP_ */
