#ifndef HOPERF_JEELIBTXFIFO_HPP_
#define HOPERF_JEELIBTXFIFO_HPP_

#include "Serial/SerialTx.hpp"
#include "ChunkedFifo.hpp"
#include "CRC.hpp"

namespace HopeRF {

using namespace Serial;

template <typename callback_t, typename target_t, int groupId = 5, int fifoSize = 32>
class JeeLibTxFifo {
public:
    enum PacketIndex {
        PREAMBLE1, PREAMBLE2, PREAMBLE3, SYNC1, SYNC2, HEADER, LENGTH, DATA, CRCLSB, CRCMSB, POSTFIX, DONE
    };

    Fifo<fifoSize> data;
    ChunkedFifoCB<callback_t, target_t> fifo;
    CRC16 crc;

    PacketIndex packetIndex = PacketIndex::PREAMBLE1;

public:
    JeeLibTxFifo(target_t &target): fifo(&data, target) {}

    ChunkedFifoCB<callback_t, target_t> &getChunkedFifo() {
        return fifo;
    }

    Writer<ChunkedFifoCB<callback_t, target_t>> out_ook(SerialConfig *type) {
        auto out = fifo.out();
        out << type;
        return out;
    }

    Writer<ChunkedFifoCB<callback_t, target_t>> out_fsk(uint8_t header) {
        auto out = fifo.out();
        SerialConfig *type = nullptr;
        out << type;
        out << header;
        return out;
    }

    /** Returns whether or not this indeed is an FSK packet, i.e. SerialConfig was nullptr calling out(). */
    bool readStart() {
        fifo.readStart();
        SerialConfig *type;
        if (fifo.in() >> type) {
            if (type == nullptr) {
                crc.reset();
                packetIndex = PacketIndex::PREAMBLE1;
                return true;
            } else {
                fifo.readAbort();
                return false;
            }
        } else {
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
                b = 0xAA; packetIndex = SYNC1; break;
            case SYNC1:
                b = 0x2D; packetIndex = SYNC2; break;
            case SYNC2:
                b = groupId;
                crc.append(b);
                packetIndex = HEADER;
                break;
            case HEADER:
                fifo.read(b);
                crc.append(b);
                packetIndex = LENGTH;
                break;
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


#endif /* HOPERF_JEELIBTXFIFO_HPP_ */
