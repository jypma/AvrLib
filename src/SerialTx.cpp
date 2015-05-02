#include "SerialTx.hpp"
#include "AtomicScope.hpp"

bool SerialConfig::prefixBit(uint8_t bitIndex) const {
    uint8_t b = prefix[bitIndex / 8];
    uint8_t bit = bitIndex % 8;
    return (b >> bit) & 1;
}

bool SerialConfig::postfixBit(uint8_t bitIndex) const {
    uint8_t b = postfix[bitIndex / 8];
    uint8_t bit = bitIndex % 8;
    return (b >> bit) & 1;
}

bool SerialConfig::hasPulseBForBit(bool bit) const {
    return (bit) ? one_b.isDefined() : zero_b.isDefined();
}

bool AbstractSerialSource::getCurrentBit() const {
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

Pulse AbstractSerialSource::getCurrentPulse() const {
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

void ChunkPulseSource::nextDataByte() {
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

void ChunkPulseSource::nextBit() {
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

Pulse ChunkPulseSource::getNextPulse() {
    AtomicScope _;

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
            if (!config->hasPulseBForBit(currentBit)) {
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

void StreamPulseSource::nextBit() {
    pulseState = SerialPulseState::A;
    switch(bitState) {
    case SerialBitState::PREFIX:
        bitIndex++;
        if (bitIndex >= config->prefix_bits) {
            bitState = SerialBitState::DATA;
            bitIndex = 0;
        }
        break;

    case SerialBitState::DATA:
        bitIndex++;
        if (bitIndex >= 8) {
            if (config->hasParity()) {
                bitIndex = 0;
                bitState = SerialBitState::PARITY_BIT;
            } else if (config->postfix_bits > 0) {
                bitIndex = 0;
                bitState = SerialBitState::POSTFIX;
            } else {
                bitState = SerialBitState::BEFORE;
            }
        }
        break;

    case SerialBitState::PARITY_BIT:
        if (config->postfix_bits > 0) {
            bitIndex = 0;
            bitState = SerialBitState::POSTFIX;
        } else {
            bitState = SerialBitState::BEFORE;
        }
        break;

    case SerialBitState::POSTFIX:
        bitIndex++;
        if (bitIndex >= config->postfix_bits) {
            bitState = SerialBitState::BEFORE;
            bitIndex = 0;
        }
        break;

    case SerialBitState::BEFORE: return;
    case SerialBitState::AFTER: return;
    }

    currentBit = getCurrentBit();
}


Pulse StreamPulseSource::getNextPulse() {
    AtomicScope _;

    if (bitState == SerialBitState::BEFORE) {
        bitIndex = 0;
        if (fifo->hasContent()) {
            fifo->read(currentByte);
            bitState = (config->prefix_bits > 0) ? SerialBitState::PREFIX : SerialBitState::DATA;
        }
        if (bitState != SerialBitState::BEFORE && bitState != SerialBitState::AFTER) {
            currentBit = getCurrentBit();
        }
    }

    Pulse result = getCurrentPulse();

    if (result.isDefined()) {
        if (pulseState == SerialPulseState::A) {
            pulseState = SerialPulseState::B;
            if (!config->hasPulseBForBit(currentBit)) {
                nextBit();
            }
        } else {
            nextBit();
        }
    } else {
        bitState = SerialBitState::BEFORE;
    }

    return result;
}
