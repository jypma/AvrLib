/*
 * VisonicDecoder.hpp
 *
 *  Created on: May 16, 2015
 *      Author: jan
 */

#ifndef VISONICDECODER_HPP_
#define VISONICDECODER_HPP_

#include "Serial/PulseCounter.hpp"
#include "Logging.hpp"

namespace Visonic {

using namespace TimeUnits;
using namespace Streams;
using namespace Serial;

class VisonicPacket: public Streams::Streamable<VisonicPacket> {
public:
    uint8_t data[5];

    typedef Format<
        Array<uint8_t, 5, &VisonicPacket::data>
    > Proto;
};

template <typename pulsecounter_t, uint8_t fifoSize = 50>
class VisonicDecoder {
public:
    uint16_t totalBits = 0;
private:
    static Logging::Log<Loggers::VisonicDecoder> log;

    typedef typename pulsecounter_t::comparator_t comparator_t;
    typedef typename pulsecounter_t::count_t count_t;

    enum class State { UNKNOWN, PREVIOUS_LONG, PREVIOUS_SHORT };

    State state = State::UNKNOWN;
    bool haveFlipped = false;
    VisonicPacket packet;
    uint8_t bit = 1;
    uint8_t pos = 0;
    Fifo<fifoSize> fifo;

    void reset() {
//        std::cout << "  reset" << std::endl;
        uint8_t bits = pos * 8;
        for (uint8_t i = 0; i < 8; i++) {
            if (bit > (1 << i)) {
                bits++;
            }
        }

        if (bits > totalBits) {
            totalBits = bits;
        }

        state = State::UNKNOWN;
        haveFlipped = false;
        bit = 1;
        pos = 0;
        noiseLength = 0;
        noisePulses = 0;
    }

    void writePacket() {
        log.timeStart();
        fifo.out() << packet;
        log.timeEnd();
    }

    void handleBit(uint8_t value) {
//        std::cout << "  got bit " << int(bit) << " of byte " << int(pos) << std::endl;

        if (value) {
            packet.data[pos] |= bit;
        } else {
            packet.data[pos] &= ~bit;
        }

        if (bit < 128) {
            bit <<= 1;
        } else {
            bit = 1;
            pos++;
        }

        if (pos >= 4 && bit >= 16) {        // 36 bits = 4 1/2 bytes of data in one packet
            packet.data[4] &= 15;           // last 4  bits of byte 5 just stay 0
//            std::cout << "  *** packet! " << std::endl;
            writePacket();
        }
    }

    void flipBits() {
        for (uint8_t i = 0; i < pos; i++) {
            packet.data[i] = ~packet.data[i];
        }
        for (uint8_t i = 1; i < bit; i <<= 1) {
            packet.data[pos] ^= i;
        }
    }

    void handleLongPulse() {
        //std::cout << "  handling long pulse in state " << int(state) << std::endl;
        switch (state) {
        case State::UNKNOWN:
            state = State::PREVIOUS_LONG;
            return;

        case State::PREVIOUS_SHORT:
            handleBit(1);
            state = State::UNKNOWN;
            return;

        case State::PREVIOUS_LONG:
            if (haveFlipped) {
                reset();
            } else {
                flipBits();
                haveFlipped = true;
                state = State::PREVIOUS_LONG;
            }
            return;
        }
    }

    void handleShortPulse() {
        //std::cout << "  handling short pulse in state " << int(state) << std::endl;
        switch (state) {
        case State::UNKNOWN:
            state = State::PREVIOUS_SHORT;
            return;

        case State::PREVIOUS_LONG:
            handleBit(0);
            state = State::UNKNOWN;
            return;

        case State::PREVIOUS_SHORT:
            if (haveFlipped) {
//                std::cout << "  two flips!" << std::endl;
                reset();
            } else {
                //std::cout << "  ***flipping" << std::endl;
                flipBits();
                haveFlipped = true;
                state = State::PREVIOUS_SHORT;
            }
            return;
        }
    }

    static constexpr count_t us_200  = (200_us).template toCounts<comparator_t>();
    static constexpr count_t us_600  = (600_us).template toCounts<comparator_t>();
    static constexpr count_t us_1000  = (1000_us).template toCounts<comparator_t>();

    count_t noiseLength = 0;
    uint8_t noisePulses = 0;
public:
    void apply(const Pulse &pulse) {
        if (pulse.isDefined()) {
            // TODO investigate whether these are actually LOw or HIGH
            //std::cout << int(evt.getType()) << " : " << int(evt.getLength()) << " , noise: " << int(noiseLength) << std::endl;
            uint16_t length = pulse.getDuration();
            if (noisePulses == 2) {
                //std::cout << "  applying noise spike" << std::endl;
                length += noiseLength;
                noiseLength = 0;
                noisePulses = 0;
            }
            if (length < us_1000) {
                if (length > us_200) {
                    noiseLength = 0;
                    noisePulses = 0;
                    if (length >= us_600) {
                        handleLongPulse();
                    } else {
                        handleShortPulse();
                    }
                } else {
                    if (noisePulses < 2) {
                        //std::cout << "  considering noise spike " << int(noisePulses) << std::endl;
                        noisePulses++;
                        noiseLength += pulse.getDuration();
                    } else {
                        //std::cout << "  too short" << std::endl;
                        reset();
                    }
                }
            } else {
                //std::cout << "  too long" << std::endl;
                reset();
            }
        } else {
            reset();
        }
    }

    inline Reader<AbstractFifo> in() {
        return fifo.in();
    }

};

}

using Visonic::VisonicDecoder;

template <typename pulsecounter_t, uint8_t fifoSize = 50>
VisonicDecoder<pulsecounter_t, fifoSize> visonicDecoder(const pulsecounter_t &counter) {
    return VisonicDecoder<pulsecounter_t, fifoSize>();
}


#endif /* VISONICDECODER_HPP_ */
