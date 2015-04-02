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

template <typename pulsecounter_t>
class IRUtils {
public:
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;

    static inline bool isMatch(const Event &event, const PulseType type, const uint16_t minLength, const uint16_t maxLength) {
        return event.getType() == type &&
               event.getLength() >= minLength &&
               event.getLength() <= maxLength;
    }

    static constexpr uint16_t min(const uint16_t us) {
        return Timer::microseconds2counts(uint32_t(us) * 60 / 100);
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
    }
};

template <typename pulsecounter_t, uint8_t fifoSize = 32>
class IRDecoder_NEC {
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
    typedef IRUtils<pulsecounter_t> Utils;
public:
    enum class State: uint8_t { Receiving, Repeat };

    State state = State::Receiving;
    uint8_t count = -1;
    uint32_t command = 0;
    Fifo<fifoSize> fifo;

    void reset() {
        count = -1;
        command = 0;
        state = State::Receiving;
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
            if (!Utils::isHigh(event, 9000)) {
                reset();
                return;
            }
        }
        if (count == 2) {
            if (Utils::isLow(event, 2250)) {
                state = State::Repeat;
                return;
            } else if (!Utils::isLow(event, 4500)) {
                reset();
                return;
            }
        }
        if (count >= 3) {
            if (count % 2 == 0) {
                if (Utils::isLow(event, 1690)) {
                    command = (command << 1) | 1;
                } else if (Utils::isLow(event, 560)) {
                    command <<= 1;
                } else {
                    reset();
                    return;
                }
            } else {
                if (!Utils::isHigh(event, 560)) {
                    reset();
                    return;
                }
            }
        }
        if (count >= 2 * 32 + 3) {
            decoded(IRType::Command);
        }
    }

    void onRepeat(const Event &event) {
        if (Utils::isHigh(event, 560)) {
            decoded(IRType::Repeat);
        } else {
            reset();
        }
    }

public:
    void apply(const Event &event) {
        switch(state) {
          case State::Receiving: onReceiving(event); break;
          case State::Repeat: onRepeat(event); break;
        }
    }

    inline Reader in() {
        return fifo.in();
    }
};


template <typename pulsecounter_t, uint8_t fifoSize = 32>
class IRDecoder_Samsung {
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
    typedef IRUtils<pulsecounter_t> Utils;
public:
    enum class State: uint8_t { Receiving, Repeat };

    State state = State::Receiving;
    uint8_t count = -1;
    uint32_t command = 0;
    Fifo<fifoSize> fifo;

    void reset() {
        count = -1;
        command = 0;
        state = State::Receiving;
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
            if (!Utils::isHigh(event, 5000)) {
                reset();
                return;
            }
        }
        if (count == 2) {
            if (Utils::isLow(event, 2250)) {
                state = State::Repeat;
                return;
            } else if (!Utils::isLow(event, 5000)) {
                reset();
                return;
            }
        }
        if (count >= 3) {
            if (count % 2 == 0) {
                if (Utils::isLow(event, 1600)) {
                    command = (command << 1) | 1;
                } else if (Utils::isLow(event, 560)) {
                    command <<= 1;
                } else {
                    reset();
                    return;
                }
            } else {
                if (!Utils::isHigh(event, 560)) {
                    reset();
                    return;
                }
            }
        }
        if (count >= 2 * 32 + 3) {
            decoded(IRType::Command);
        }
    }

    void onRepeat(const Event &event) {
        if (Utils::isHigh(event, 560)) {
            decoded(IRType::Repeat);
        } else {
            reset();
        }
    }

public:
    void apply(const Event &event) {
        switch(state) {
          case State::Receiving: onReceiving(event); break;
          case State::Repeat: onRepeat(event); break;
        }
    }

    inline Reader in() {
        return fifo.in();
    }
};


#endif
