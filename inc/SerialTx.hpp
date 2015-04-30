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

    bool prefixBit(uint8_t bitIndex) const {
        uint8_t b = prefix[bitIndex / 8];
        uint8_t bit = bitIndex % 8;
        return (b >> bit) & 1;
    }

    bool postfixBit(uint8_t bitIndex) const {
        uint8_t b = postfix[bitIndex / 8];
        uint8_t bit = bitIndex % 8;
        return (b >> bit) & 1;
    }
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

    bool getCurrentBit() {
        switch(bitState) {
          case SerialBitState::PREFIX: return config->prefixBit(bitIndex);
          case SerialBitState::DATA: return (config->bitOrder == SerialBitOrder::LSB_FIRST) ?
                                            (currentByte >> bitIndex) & 1 :
                                            (currentByte << bitIndex) & 128;
          case SerialBitState::PARITY_BIT:
              return (config->parity == SerialParity::EVEN) ?
                                           parity_even_bit(currentByte) :
                                          !parity_even_bit(currentByte);
          case SerialBitState::POSTFIX: return config->postfixBit(bitIndex);
          default: return 0;
        }
        return 0;
    }
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

    void nextDataByte() {
        bitIndex = 0;
        pulseState = SerialPulseState::A;
        if (fifo->hasReadAvailable()) {
            fifo->read(currentByte);
            bitState = SerialBitState::DATA;
        } else {
            bitState = (config->hasParity() && bitState == SerialBitState::DATA) ? SerialBitState::PARITY_BIT :
                       (config->postfix_bits > 0) ? SerialBitState::POSTFIX : SerialBitState::AFTER;
        }
    }

    Pulse getCurrentPulse() {
        if (bitState == SerialBitState::BEFORE || bitState == SerialBitState::AFTER) {
            return Pulse::empty();
        } else {
            if (pulseState == SerialPulseState::A) {
                if (currentBit != 0) {
                    return config->one_a;
                } else {
                    return config->zero_a;
                }
            } else {
                if (currentBit != 0) {
                    return config->one_b;
                } else {
                    return config->zero_b;
                }
            }
        }
    }

    void nextBit() {
        pulseState = SerialPulseState::A;
        switch(bitState) {
        case SerialBitState::PREFIX:
            bitIndex++;
            if (bitIndex >= config->prefix_bits) {
                bitState = SerialBitState::DATA;
                bitIndex = 0;
                nextDataByte();
            }
            break;

        case SerialBitState::DATA:
            bitIndex++;
            if (bitIndex >= 8) {
                if (config->hasParity()) {
                    bitIndex = 0;
                    bitState = SerialBitState::PARITY_BIT;
                } else {
                    nextDataByte();
                }
            }
            break;

        case SerialBitState::PARITY_BIT:
            nextDataByte();
            //bitIndex = 0;
            //bitState = (config->postfix_bits > 0) ? SerialBitState::POSTFIX : SerialBitState::AFTER;
            break;

        case SerialBitState::POSTFIX:
            bitIndex++;
            if (bitIndex >= config->postfix_bits) {
                bitState = SerialBitState::AFTER;
                bitIndex = 0;
            }
            break;

        case SerialBitState::BEFORE: return;
        case SerialBitState::AFTER: return;
        }

        currentBit = getCurrentBit();
    }


public:
    ChunkPulseSource(ChunkedFifo *_fifo): fifo(_fifo) {}

    Pulse getNextPulse() {
        if (bitState == SerialBitState::BEFORE) {
            bitIndex = 0;
            fifo->readStart();
            if (fifo->in() >> config && config != nullptr) {
                if (config->prefix_bits > 0) {
                    bitState = SerialBitState::PREFIX;
                } else {
                    if (fifo->hasReadAvailable()) {
                        nextDataByte();
                    }
                }
            } else {
                // this is not good. We should have always gotten the config pointer first. Let's just drop this chunk.
                fifo->readEnd();
            }

            if (bitState != SerialBitState::BEFORE && bitState != SerialBitState::AFTER) {
                currentBit = getCurrentBit();
            }
        }

        Pulse result = getCurrentPulse();
        if (result.isDefined()) {
            if (pulseState == SerialPulseState::A) {
                pulseState = SerialPulseState::B;
                if ((currentBit == 0 && config->zero_b.isDefined())
                 || (currentBit != 0 && config->one_b.isDefined())) {

                } else {
                    nextBit();
                }
            } else {
                nextBit();
            }
        } else {
            fifo->readEnd();
            bitState = SerialBitState::BEFORE;
        }

        return result;
    }

};

#endif /* SERIALTX_HPP_ */
