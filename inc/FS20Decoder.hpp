#ifndef FS20DECODER_H
#define FS20DECODER_H

#include "PulseCounter.hpp"
#include "FS20Packet.hpp"
#include <util/parity.h>

namespace FS20 {

using namespace TimeUnits;

template <typename pulsecounter_t, uint8_t fifoSize = 32>
class FS20Decoder {
    typedef typename pulsecounter_t::comparator_t comparator_t;
    typedef typename pulsecounter_t::count_t count_t;
    typedef PulseEvent<count_t> Event;

public:
    static constexpr count_t zero_length  = (400_us).template toCounts<comparator_t>();
    static constexpr count_t one_length  = (600_us).template toCounts<comparator_t>();

    enum class State { SYNC, RECEIVING };

    static inline bool isZero(count_t length) {
        return length > count_t(0.75*zero_length) && length <= count_t((zero_length + one_length) / 2);
    }

    static inline bool isOne(count_t length) {
        return length > count_t((zero_length + one_length) / 2) && length < count_t(1.25*one_length);
    }
    State state = State::SYNC;
    count_t lastLength = 0;
    int8_t bitCount = -1;
    uint8_t byteCount = 0;
    FS20Packet packet;
    bool parityError = false;
    Fifo<fifoSize> fifo;

    uint8_t log[255] = {};
    uint8_t logIdx = 0;
    bool logOn = true;

    inline uint8_t *currentByte() {
        return (uint8_t *)(&packet) + byteCount;
    }

    void reset() {
        if (byteCount > 0) {
            logOn = false;
            if (logIdx < 250) {
                log[logIdx] = uint8_t(state);
                logIdx++;
                log[logIdx] = byteCount;
                logIdx++;
                log[logIdx] = bitCount;
            }
        }
        lastLength = 0;
        bitCount = -1;
        byteCount = 0;
        packet = FS20Packet();
        parityError = false;
        state = State::SYNC;
        logIdx = 0;
    }

    void checkParity(uint8_t value, uint8_t bit) {
        auto parity = parity_even_bit(value);
        if (parity != bit) {
            parityError = true;
        }
    }

    void packetComplete() {
        if (!parityError && packet.isChecksumCorrect()) {
            fifo.out() << packet;
        }
        reset();
    }

    void applyBitSync(uint8_t bit) {
        if (bit == 0) {
            bitCount++;
            if (bitCount > 12) {
                // More than 12 bits zero seen is still zero.
                bitCount = 12;
            }
        } else {
            if (bitCount > 9) {
                state = State::RECEIVING;
                bitCount = 0;
            } else {
                reset();
            }
        }
    }

    void applyBitReceiving(uint8_t bit) {
        bitCount++;
        if (bitCount == 9) {
            checkParity(*currentByte(), bit);
            bitCount = 0;
            byteCount++;
            if ((byteCount == 4) && !packet.hasCommandExt()) {
                byteCount++;
            }
            if (byteCount >= 6) {
                packetComplete();
            }
        } else {
            *currentByte() = (*currentByte() << 1) | bit;
        }
    }

    void applyBit(uint8_t bit) {
        switch (state) {
        case State::SYNC: applyBitSync(bit); break;
        case State::RECEIVING: applyBitReceiving(bit);  break;
        }
        lastLength = 0;
    }

public:
    void apply(const Event &evt) {
        if (logOn) {
            log[logIdx] = evt.getLength();
            logIdx++;
            if (logIdx > 250) {
                logOn = false;
            }
        }
        switch(evt.getType()) {
        case PulseType::LOW:
            if (isZero(evt.getLength())) {
                if (isZero(lastLength)) {
                    applyBit(0);
                } else {
                    reset();
                }
            } else if (isOne(evt.getLength())) {
                if (isOne(lastLength)) {
                    applyBit(1);
                } else {
                    reset();
                }
            } else {
                reset();
            }
            break;

        case PulseType::HIGH:
            lastLength = evt.getLength();
            break;

        case PulseType::TIMEOUT:
            reset();
            break;
        }
    }

    inline Reader in() {
        return fifo.in();
    }
};

}

using FS20::FS20Decoder;

template <typename pulsecounter_t, uint8_t fifoSize = 32>
FS20Decoder<pulsecounter_t, fifoSize> fs20Decoder(const pulsecounter_t &counter) {
    return FS20Decoder<pulsecounter_t, fifoSize>();
}
#endif
