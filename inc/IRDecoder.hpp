#ifndef IRDECODER_H
#define IRDECODER_H

#include "PulseCounter.hpp"

enum IRType: uint8_t { Command, Repeat };

class IRCode {
    IRType type;
    uint32_t command;
public:
    IRCode(IRType _type, uint32_t _command): type(_type), command(_command) {}
    IRCode(): type(IRType::Command), command(0) {}

    inline IRType getType() const {
        return type;
    }

    inline uint32_t getCommand() const {
        return command;
    }

    static void write(Writer &out, const IRCode &code) {
        out << code.type;
        if (code.type == IRType::Command) {
            out << code.command;
        }
    }
    static void read(Reader &in, IRCode &code) {
        if (in >> code.type) {
            if (code.type == IRType::Command) {
                in >> code.command;
            }
        }
    }
};

template <typename pulsecounter_t, uint8_t fifoSize = 32>
class IRDecoder_NEC {
public:
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
    enum class State: uint8_t { Receiving, Error, Repeat };

    uint8_t count = -1;
    State state = State::Receiving;
    uint32_t command = 0;
    Fifo<fifoSize> fifo;

    static inline bool isMatch(const Event &event, const PulseType type, const uint16_t minLength, const uint16_t maxLength) {
        return event.getType() == type &&
               event.getLength() >= minLength &&
               event.getLength() <= maxLength;
    }

    static constexpr uint16_t min(const uint16_t us) {
        return Timer::microseconds2counts(uint32_t(us) * 70 / 100);
    }

    static constexpr uint16_t max(const uint16_t us) {
        return Timer::microseconds2counts(uint32_t(us) * 125 / 100);
    }

    static inline bool isHigh(const Event &event, const uint16_t expectedLength) {
        return isMatch(event, PulseType::HIGH, min(expectedLength), max(expectedLength));
    }

    static inline bool isLow(const Event &event, const uint16_t expectedLength) {
        return isMatch(event, PulseType::LOW, min(expectedLength), max(expectedLength));
    }

    void reset() {
        count = -1;
        state = State::Receiving;
        command = 0;
    }

    void decoded(IRType type) {
        fifo.out() << IRCode(type, command);
        reset();
    }

    void onReceiving(const Event &event) {
        count++;

        if (count == 0) {
            if (event.getType() == PulseType::LOW) {
                return;
            } else {
                count = -1;
                return;
            }
        }
        if (count == 1) {
            if (!isHigh(event, 9000)) {
                state = State::Error;
                return;
            }
        }
        if (count == 2) {
            if (isLow(event, 2250)) {
                state = State::Repeat;
                return;
            } else if (!isLow(event, 4500)) {
                state = State::Error;
                return;
            }
        }
        if (count >= 3) {
            if (count % 2 == 0) {
                if (isLow(event, 1690)) {
                    command = (command << 1) | 1;
                } else if (isLow(event, 560)) {
                    command <<= 1;
                } else {
                    state = State::Error;
                    return;
                }
            } else {
                if (!isHigh(event, 560)) {
                    state = State::Error;
                    return;
                }
            }
        }
        if (count >= 2 * 32 + 3) {
            decoded(IRType::Command);
        }
    }

    void onRepeat(const Event &event) {
        if (isHigh(event, 560)) {
            decoded(IRType::Repeat);
        } else {
            state = State::Error;
        }
    }

    void onError(const Event &event) {
        if (event.getType() == PulseType::TIMEOUT) {
            state = State::Receiving;
            count = -1;
        }
    }

public:
    void apply(const Event &event) {
        switch(state) {
          case State::Receiving: onReceiving(event); break;
          case State::Repeat: onRepeat(event); break;
          case State::Error: onError(event); break;
        }
    }

    inline Reader in() {
        return fifo.in();
    }
};

#endif
