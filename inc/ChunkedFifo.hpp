/*
 * ChunkedFifo.hpp
 *
 *  Created on: Jan 21, 2015
 *      Author: jan
 */

#ifndef CHUNKEDFIFO_HPP_
#define CHUNKEDFIFO_HPP_

#include "Fifo.hpp"
#include "Reader.hpp"
#include "Writer.hpp"

class ChunkedFifo {
    const static Writer::VTable writerVTable;
    static void writeStart(void *delegate);
    static void writeCommit(void *delegate);
    static void writeRollback(void *delegate);
    static bool write(void *delegate, uint8_t b);
    static bool isWriting(void *delegate);

    const static Reader::VTable readerVTable;
    static void readStart(void *delegate);
    static void readCommit(void *delegate);
    static void readRollback(void *delegate);
    static bool read(void *delegate, uint8_t &b);
    static uint8_t getAvailable(void *delegate);
    static bool isReading(void *delegate);

    AbstractFifo *data;

    volatile uint8_t *writeLengthPtr = nullptr;
    bool writeValid = false;

    uint8_t readLength = 0;
    bool readValid = false;

public:
    ChunkedFifo(AbstractFifo *_data): data(_data) {}

    void clear();

    bool isFull() const;

    inline bool hasContent() const {
        return data->hasContent();
    }

    void writeStart();

    inline bool isWriting() const {
        return data->isWriteMarked();
    }

    bool write (uint8_t b);

    void writeEnd();

    inline void writeAbort() {
        data->resetWrite();
    }

    void readStart();

    inline bool isReading() const {
        return data->isReading();
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

    void readEnd();

    void readAbort();

    Writer out();

    Reader in();
};

#endif /* CHUNKEDFIFO_HPP_ */
