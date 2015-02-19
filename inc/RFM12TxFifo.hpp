/*
 * RFM12TxFifo.hpp
 *
 *  Created on: Jan 26, 2015
 *      Author: jan
 */

#ifndef RFM12TXFIFO_HPP_
#define RFM12TXFIFO_HPP_

#include "ChunkedFifo.hpp"
#include "CRC.hpp"

class RFM12TxFifo {
    enum PacketIndex {
        PREAMBLE1, PREAMBLE2, SYNC, LENGTH, DATA, CRCLSB, CRCMSB, POSTFIX
    };

    ChunkedFifo *fifo; // TODO THIS could be template, since there'll only be 1 RFM12TxFifo instance anyways.
    CRC16 crc;

    PacketIndex packetIndex = PacketIndex::PREAMBLE1;

public:
    RFM12TxFifo(ChunkedFifo *_fifo): fifo(_fifo) {}

    inline Writer out() {
        return fifo->out();
    }

    void readStart() {
        fifo->readStart();
        crc.reset();
        packetIndex = PacketIndex::PREAMBLE1;
    }

    inline uint8_t isReading() const {
        return fifo->isReading() && packetIndex != POSTFIX;
    }

    void read(uint8_t &b) {
        switch(packetIndex) {
            case PREAMBLE1:
                b = 0xAA; packetIndex = PREAMBLE2; break;
            case PREAMBLE2:
                b = 0xAA; packetIndex = SYNC; break;
            case SYNC:
                b = 0x2D; packetIndex = LENGTH; break;
            case LENGTH:
                b = fifo->getReadAvailable();
                crc.append(b);
                packetIndex = (b > 0) ? DATA : CRCLSB;
                break;
            case DATA:
                fifo->read(b);
                crc.append(b);
                if (fifo->getReadAvailable() == 0) packetIndex = CRCLSB;
                break;
            case CRCLSB:
                b = (uint8_t)(crc.get()); packetIndex = CRCMSB; break;
            case CRCMSB:
                b = (uint8_t)(crc.get() >> 8); packetIndex = POSTFIX; break;
            case POSTFIX:
                b = 0xAA; break;
        }
    }

    void readAbort() {
        fifo->readAbort();
    }

    void readEnd() {
        fifo->readEnd();
    }

    inline bool hasContent() const {
        return fifo->hasContent();
    }

};

#endif /* RFM12TXFIFO_HPP_ */
