/*
 * SerialTx.hpp
 *
 *  Created on: Apr 11, 2015
 *      Author: jan
 */

#ifndef SERIALTX_HPP_
#define SERIALTX_HPP_

#include <util/parity.h>
#include "InterruptHandler.hpp"
#include "Serial/PulseTx.hpp"
#include "ChunkedFifo.hpp"

namespace Serial {

enum class SerialParity {
    NONE, EVEN, ODD
};

enum class SerialBitOrder {
    LSB_FIRST, MSB_FIRST
};

struct SerialConfig {
    bool highOnIdle;

    const uint8_t *prefix;   /* Prefix bits are ALWAYS sent out LSB first, regardless of the configured data bitOrder */
    uint8_t prefix_bits;

    Pulse zero_a;
    Pulse zero_b;
    Pulse one_a;
    Pulse one_b;

    SerialParity parity;
    SerialBitOrder bitOrder;

    const uint8_t *postfix; /* Postfix bits are ALWAYS sent out LSB first, regardless of the configured data bitOrder */
    uint8_t postfix_bits;

    bool hasParity() {
        return parity != SerialParity::NONE;
    }

    bool prefixBit(uint8_t bitIndex) const;

    bool postfixBit(uint8_t bitIndex) const;

    bool hasPulseBForBit(bool bit) const;
};

//TODO should be private. We could do without AFTER.
enum class SerialBitState: uint8_t {
    BEFORE, PREFIX, DATA_LSB, DATA_MSB, PARITY_BIT, POSTFIX, AFTER
};
enum class SerialPulseState: uint8_t {
    A, B
};

class AbstractSerialSource {
protected:
    SerialConfig *config = nullptr;

    SerialBitState bitState = SerialBitState::BEFORE;
    uint8_t bitIndex = 0;
    bool currentBit = false;
    SerialPulseState pulseState = SerialPulseState::A;
    uint8_t currentByte, currentByteBits = 0;

    bool getCurrentBit();

    Pulse getCurrentPulse() const;
public:
    bool isHighOnIdle() {
        return config->highOnIdle;
    }
};

/**
 * Outputs a single chunk at a time, from a chunked fifo. Every chunk in the fifo
 * should start with a pointer to a SerialConfig settings for that chunk; the remaining
 * bytes are the serial data to send.
 */
class ChunkPulseSource: public AbstractSerialSource {
    ChunkedFifo *fifo;

    void nextDataByte();

    void nextBit();

public:
    ChunkPulseSource(ChunkedFifo &_fifo): fifo(&_fifo) {}

    Pulse getNextPulse();
};

/**
 * Outputs a single byte at a time. from a fifo. The serial config is set at
 * construction time.
 */
class StreamPulseSource: public AbstractSerialSource {
    AbstractFifo *fifo;

    void nextBit();
public:
    StreamPulseSource(AbstractFifo &_fifo, SerialConfig &_config): fifo(&_fifo) {
        config = &_config;
    }

    Pulse getNextPulse();
};

}

#endif /* SERIALTX_HPP_ */
