#ifndef HOPERF_JEELIBRXFIFO_HPP_
#define HOPERF_JEELIBRXFIFO_HPP_

#include "Fifo.hpp"
#include "ChunkedFifo.hpp"
#include "CRC.hpp"
#include "Streams/Reader.hpp"

namespace HopeRF{

enum class JeeLibState: uint8_t { header, length, data };

template <int groupId = 5, int fifoSize = 32, bool checkCrc = true>
class JeeLibRxFifo {
    Fifo<fifoSize> data;
    ChunkedFifo fifo = { &data };
    CRC16 crc;
    JeeLibState state;
    uint8_t remaining;

    static constexpr uint8_t MAX_LENGTH = 64;

public:
    void writeStart(uint8_t b) {
        if (b != groupId) {
            return;
        }

        crc.reset();
        crc.append(b);
        state = JeeLibState::header;
        fifo.writeStart();
    }

    void write(uint8_t b) {
        switch(state) {
        case JeeLibState::header:
            crc.append(b);
            fifo.write(b);
            state = JeeLibState::length;
            break;
        case JeeLibState::length:
            crc.append(b);
            remaining = b + 2; // 2 extra bytes for CRC
            if (remaining > MAX_LENGTH) {
                remaining = MAX_LENGTH;
            }
            state = JeeLibState::data;
            break;
        case JeeLibState::data:
            if (remaining > 0) {
                if (remaining > 2) {
                    fifo.write(b);
                }
                crc.append(b);
                remaining--;
                if (remaining == 0) {
                    if (crc.isValid()) {
                        fifo.writeEnd();
                    } else if (!checkCrc) {
                        fifo.writeEnd();
                    } else {
                        fifo.writeAbort();
                    }
                }
            }
            break;
        }
    }

    inline bool isWriting() const {
        return fifo.isWriting();
    }

    inline void writeAbort() {
        fifo.writeAbort();
    }

    inline Streams::Reader<ChunkedFifo> in() {
        return fifo.in();
    }

    inline bool hasContent() const {
        return fifo.hasContent();
    }
};

}

#endif /* HOPERF_JEELIBRXFIFO_HPP_ */
