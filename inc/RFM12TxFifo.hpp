/*
 * RFM12TxFifo.hpp
 *
 *  Created on: Jan 26, 2015
 *      Author: jan
 */

#ifndef RFM12TXFIFO_HPP_
#define RFM12TXFIFO_HPP_

#include "SerialTx.hpp"
#include "ChunkedFifo.hpp"
#include "CRC.hpp"

template <int fifoSize = 32>
class RFM12TxFifo {
    enum PacketIndex {
        PREAMBLE1, PREAMBLE2, SYNC, LENGTH, DATA, CRCLSB, CRCMSB, POSTFIX, DONE
    };

    Fifo<fifoSize> data;
    ChunkedFifo fifo = { &data };
    CRC16 crc;

    PacketIndex packetIndex = PacketIndex::PREAMBLE1;
    SerialConfig *type;

public:
    Writer out(SerialConfig *type) {
        Writer out = fifo.out();
        for (uint8_t i = 0; i < sizeof(SerialConfig*); i++) {
            out << ((uint8_t*)(&type))[i];
        }
        return out;
    }

    SerialConfig *readStart() {
        fifo.readStart();
        for (uint8_t i = 0; i < sizeof(SerialConfig*); i++) {
            fifo.read(*((uint8_t*)(&type) + i));
        }
        crc.reset();
        packetIndex = (type == nullptr) ? PacketIndex::PREAMBLE1 : PacketIndex::DATA;
        return type;
    }

    inline uint8_t isReading() const {
        return fifo.isReading() && (packetIndex != DONE);
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
                b = fifo.getReadAvailable();
                crc.append(b);
                packetIndex = (b > 0) ? DATA : CRCLSB;
                break;
            case DATA:
                fifo.read(b);
                if (type == nullptr) {
                    crc.append(b);
                    if (fifo.getReadAvailable() == 0) packetIndex = CRCLSB;
                } else {
                    if (fifo.getReadAvailable() == 0) packetIndex = DONE;
                }
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
    }

    void readAbort() {
        fifo.readAbort();
    }

    void readEnd() {
        fifo.readEnd();
    }

    inline bool hasContent() const {
        return fifo.hasContent();
    }

};

#endif /* RFM12TXFIFO_HPP_ */
