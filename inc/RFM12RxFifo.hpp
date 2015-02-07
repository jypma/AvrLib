/*
 * RFM12RxFifo.hpp
 *
 *  Created on: Jan 27, 2015
 *      Author: jan
 */

#ifndef RFM12RXFIFO_HPP_
#define RFM12RXFIFO_HPP_

#include "ChunkedFifo.hpp"
#include "CRC.hpp"
#include "Reader.hpp"

class RFM12RxFifo {
    ChunkedFifo *fifo; // TODO THIS should be template, since there'll only be 1 RFM12TxFifo instance anyways.
    CRC16 crc;
    uint8_t length = 0;

public:
    RFM12RxFifo(ChunkedFifo *_fifo): fifo(_fifo) {}

    void writeStart(uint8_t _length) {
        length = _length + 2;
        crc.reset();
        crc.append(_length);
        fifo->writeStart();
    }

    void write(uint8_t b) {
        if (length > 0) {
            if (length > 2) {
                fifo->write(b);
            }
            crc.append(b);
            length--;
            if (length == 0) {
                if (crc.isValid()) {
                    fifo->writeEnd();
                } else {
                    fifo->writeAbort();
                }
            }
        }
    }

    inline bool isWriting() const {
        return fifo->isWriting();
    }

    inline void writeAbort() {
        fifo->writeAbort();
    }

    inline Reader in() {
        return fifo->in();
    }

    inline bool hasContent() const {
        return fifo->hasContent();
    }
};



#endif /* RFM12RXFIFO_HPP_ */
