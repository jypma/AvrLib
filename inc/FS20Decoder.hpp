#ifndef FS20DECODER_H
#define FS20DECODER_H

#include "PulseCounter.hpp"
#include <util/parity.h>

struct FS20Packet {
    uint8_t houseCodeHi = 0;
    uint8_t houseCodeLo = 0;
    uint8_t address = 0;
    uint8_t command = 0;
    uint8_t commandExt = 0;
    uint8_t checksum = 0;

    bool hasCommandExt() const;
    uint8_t getExpectedChecksum() const;
    bool isChecksumCorrect() const;

    static void write(Writer &out, const FS20Packet &packet) {
        out << packet.houseCodeHi << packet.houseCodeLo << packet.address << packet.command;
        if (packet.hasCommandExt()) {
            out << packet.commandExt;
        }
    }
    static void read(Reader &in, FS20Packet &packet) {
        in >> packet.houseCodeHi >> packet.houseCodeLo >> packet.address >> packet.command;
        if (packet.hasCommandExt()) {
            in >> packet.commandExt;
        }
        packet.checksum = packet.getExpectedChecksum();
    }
};

template <typename pulsecounter_t, uint8_t fifoSize = 32>
class FS20Decoder {
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
    typedef typename pulsecounter_t::count_t count_t;

public:
    static constexpr count_t zero_length = Timer::microseconds2counts(400); // 256: 24     64: 100
    static constexpr count_t one_length = Timer::microseconds2counts(600);  // 256: 37     64: 150

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
        case PulseType::HIGH:
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

        case PulseType::LOW:
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

#endif
