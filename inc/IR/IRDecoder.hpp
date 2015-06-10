#ifndef IRDECODER_H
#define IRDECODER_H

#include "Serial/PulseCounter.hpp"

namespace IR {

using namespace Streams;
using namespace Serial;

enum IRType: uint8_t { Command, Repeat };

class IRCode: public Streamable<IRCode> {
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

    inline bool isCommand() const {
        return type == IRType::Command;
    }

    typedef Format<
        Scalar<IRType, &IRCode::type>,
        Conditional<&IRCode::isCommand,
            Scalar<uint32_t, &IRCode::command>
        >
    > Proto;

};

template <typename pulsecounter_t>
class IRUtils {
public:
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;

    static bool isMatch(const Event &event, const PulseType type, const uint16_t minLength, const uint16_t maxLength) {
        return event.getType() == type &&
               event.getLength() >= minLength &&
               event.getLength() <= maxLength;
    }

    template <typename Value>
    static constexpr uint16_t min() {
        return Value::template percent<60>().template toCounts<Timer>();
    }

    template <typename Value>
    static constexpr uint16_t max() {
        return Value::template percent<125>().template toCounts<Timer>();
    }

    template <typename Value>
    static bool isHigh(const Event &event, Value value) {
        return isMatch(event, PulseType::LOW, min<Value>(), max<Value>());
    }

    template <typename Value>
    static bool isLow(const Event &event, Value value) {
        return isMatch(event, PulseType::HIGH,  min<Value>(), max<Value>());
    }

    void reset() {
    }
};

template <typename pulsecounter_t, uint8_t fifoSize = 32>
class IRDecoder_NEC: public IRUtils<pulsecounter_t> {
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
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
        using namespace TimeUnits;

        count++;

        if (count == 0) {
            if (event.getType() == PulseType::HIGH) {
                return;
            } else {
                count = -1;
                return;
            }
        }
        if (count == 1) {
            if (!this->isHigh(event, 9000_us)) {
                reset();
                return;
            }
        }
        if (count == 2) {
            if (this->isLow(event, 2250_us)) {
                state = State::Repeat;
                return;
            } else if (!this->isLow(event, 4500_us)) {
                reset();
                return;
            }
        }
        if (count >= 3) {
            if (count % 2 == 0) {
                if (this->isLow(event, 1690_us)) {
                    command = (command << 1) | 1;
                } else if (this->isLow(event, 560_us)) {
                    command <<= 1;
                } else {
                    reset();
                    return;
                }
            } else {
                if (!this->isHigh(event, 560_us)) {
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
        using namespace TimeUnits;

        if (this->isHigh(event, 560_us)) {
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

    inline Streams::Reader<AbstractFifo> in() {
        return fifo.in();
    }
};


template <typename pulsecounter_t, uint8_t fifoSize = 32>
class IRDecoder_Samsung: public IRUtils<pulsecounter_t> {
    typedef typename pulsecounter_t::PulseEvent Event;
    typedef typename pulsecounter_t::Timer Timer;
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
        using namespace TimeUnits;

        count++;
        if (count == 0) {
            if (event.getType() == PulseType::HIGH) {
                return;
            } else {
                count = -1;
                return;
            }
        }
        if (count == 1) {
            if (!this->isHigh(event, 5000_us)) {
                reset();
                return;
            }
        }
        if (count == 2) {
            if (this->isLow(event, 2250_us)) {
                state = State::Repeat;
                return;
            } else if (!this->isLow(event, 5000_us)) {
                reset();
                return;
            }
        }
        if (count >= 3) {
            if (count % 2 == 0) {
                if (this->isLow(event, 1600_us)) {
                    command = (command << 1) | 1;
                } else if (this->isLow(event, 560_us)) {
                    command <<= 1;
                } else {
                    reset();
                    return;
                }
            } else {
                if (!this->isHigh(event, 560_us)) {
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
        using namespace TimeUnits;

        if (this->isHigh(event, 560_us)) {
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

    inline Reader<AbstractFifo> in() {
        return fifo.in();
    }
};

}

using IR::IRCode;
using IR::IRDecoder_NEC;
using IR::IRDecoder_Samsung;
using IR::IRType;


#endif
