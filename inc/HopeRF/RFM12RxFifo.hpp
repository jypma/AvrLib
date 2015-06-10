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
#include "Streams/Reader.hpp"

namespace HopeRF{

template <int fifoSize = 32, bool checkCrc = true>
class RFM12RxFifo {
    Fifo<fifoSize> data;
    ChunkedFifo fifo = { &data };
    CRC16 crc;
    uint8_t remaining = 0;

public:
    /**
     * @param _length Number of data bytes in the packet. Excludes the 2 CRC bytes that follow after data.
     */
    void writeStart(uint8_t length) {
        if (length > 63) {
            length = 63; // RFM12 doesn't do packets longer than this, so the current "packet" probably is gonna fail.
        }
        remaining = length + 2; // remaining are [length] data bytes and 2 CRC bytes.
        crc.reset();
        crc.append(length);
        fifo.writeStart();
    }

    void write(uint8_t b) {
        if (remaining > 0) {
            if (remaining > 2) {
                fifo.write(b);
            }
            crc.append(b);
            remaining--;
            if (remaining == 0) {
                if (crc.isValid() || !checkCrc) {
                    fifo.writeEnd();
                } else {
                    fifo.writeAbort();
                }
            }
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

#endif /* RFM12RXFIFO_HPP_ */
