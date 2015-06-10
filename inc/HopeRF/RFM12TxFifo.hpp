/*
 * RFM12TxFifo.hpp
 *
 *  Created on: Jan 26, 2015
 *      Author: jan
 */

#ifndef RFM12TXFIFO_HPP_
#define RFM12TXFIFO_HPP_

#include "Serial/SerialTx.hpp"
#include "ChunkedFifo.hpp"
#include "CRC.hpp"

namespace HopeRF {

using namespace Serial;

template <int fifoSize = 32>
class RFM12TxFifo {
    enum PacketIndex {
        PREAMBLE1, PREAMBLE2, PREAMBLE3, SYNC, LENGTH, DATA, CRCLSB, CRCMSB, POSTFIX, DONE
    };

    Fifo<fifoSize> data;
    ChunkedFifo fifo = { &data };
    CRC16 crc;

    PacketIndex packetIndex = PacketIndex::PREAMBLE1;

public:
    ChunkedFifo &getChunkedFifo() {
        return fifo;
    }

    Writer<ChunkedFifo> out(SerialConfig *type) {
        auto out = fifo.out();
        out << type;
        return out;
    }

    /** Returns whether or not this indeed is an RFM12 packet, i.e. SerialConfig was nullptr calling out(). */
    bool readStart() {
        fifo.readStart();
        SerialConfig *type;
        fifo.in() >> type;
        if (type == nullptr) {
            crc.reset();
            packetIndex = (type == nullptr) ? PacketIndex::PREAMBLE1 : PacketIndex::DATA;
            return true;
        } else {
            fifo.readAbort();
            return false;
        }
    }

    inline bool hasReadAvailable() const {
        return fifo.isReading() && (packetIndex != DONE);
    }

    void read(uint8_t &b) {
        switch(packetIndex) {
            case PREAMBLE1:
                b = 0xAA; packetIndex = PREAMBLE2; break;
            case PREAMBLE2:
                b = 0xAA; packetIndex = PREAMBLE3; break;
            case PREAMBLE3:
                b = 0xAA; packetIndex = SYNC; break;
            case SYNC:
                b = 0x2D; packetIndex = LENGTH; break;
            case LENGTH:
                b = fifo.getReadAvailable();
                crc.append(b);
                packetIndex = (b > 0) ? DATA : CRCLSB;
                break;
            case DATA:
                fifo.read(b);
                crc.append(b);
                if (fifo.getReadAvailable() == 0) packetIndex = CRCLSB;
                break;
            case CRCLSB:
                b = (uint8_t)(crc.get()); packetIndex = CRCMSB; break;
            case CRCMSB:
                b = (uint8_t)(crc.get() >> 8); packetIndex = POSTFIX; break;
            case POSTFIX:
                b = 0xAA; packetIndex = DONE; break;
            case DONE:
                b = 0xAA; break;
        }

        if (packetIndex == DONE) {
            fifo.readEnd();
        }
    }

    void readAbort() {
        fifo.readAbort();
    }

    inline bool hasContent() const {
        return fifo.hasContent();
    }

};

}

#endif /* RFM12TXFIFO_HPP_ */
