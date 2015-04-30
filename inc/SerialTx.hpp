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
#include "AtomicScope.hpp"
#include "PulseTx.hpp"
#include "ChunkedFifo.hpp"

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
};

enum class SerialBitState: uint8_t {
    BEFORE, PREFIX, DATA, PARITY_BIT, POSTFIX, AFTER
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
    uint8_t currentByte = 0;

    bool getCurrentBit();
public:
    bool isHighOnIdle() {
        return config->highOnIdle;
    }
};

/**
 * Outputs a single chunk from a chunked fifo.
 */
class ChunkPulseSource: public AbstractSerialSource {
    ChunkedFifo *fifo;

    void nextDataByte();

    Pulse getCurrentPulse() const;

    void nextBit();

public:
    ChunkPulseSource(ChunkedFifo *_fifo): fifo(_fifo) {}

    Pulse getNextPulse();
};

#endif /* SERIALTX_HPP_ */
