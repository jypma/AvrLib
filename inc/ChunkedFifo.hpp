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
    static void writeEnd(void *delegate);
    static bool write(void *delegate, uint8_t b);

    const static Reader::VTable readerVTable;
    static void readStart(void *delegate);
    static void readEnd(void *delegate);
    static bool read(void *delegate, uint8_t &b);
    static uint8_t getAvailable(void *delegate);

    AbstractFifo *data;
    AbstractFifo *lengths;

    uint8_t writeLength = 0;
    bool writeValid = false;

    uint8_t readLength = 0;
    bool readValid = false;

public:
    ChunkedFifo(AbstractFifo *_data, AbstractFifo *_lengths): data(_data), lengths(_lengths) {}

    void clear();

    bool isFull() const;

    inline bool hasContent() const {
        return lengths->hasContent();
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
        return data->isReadMarked();
    }

    inline uint8_t getReadAvailable() {
        return readLength;
    }

    bool read(uint8_t &ch);

    void readEnd();

    void readAbort();

    Writer out();

    Reader in();
};

#endif /* CHUNKEDFIFO_HPP_ */
