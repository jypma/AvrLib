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
#include "Streams/Protocol.hpp"

namespace Visonic {

using namespace Time;
using namespace Streams;
using namespace Serial;

class VisonicPacket {
public:
    uint8_t data[5];
    uint8_t lastBit;
    bool flipped;

    /**
     * Pads data[4] after a received packet completes unsuccessfully, assuming that
     * we in fact missed the first bits.
     *
     * data[0] will be padded with (4 - lastBit) LSB's.
     * @param bits The number of bits that we did receive for data[4] (0..3)
     */
    void pad(const uint8_t lastBit) {
        this->lastBit = lastBit;
        if (lastBit >= 4) return;

        const uint8_t left = (4 - lastBit);
        const uint8_t right = (8 - (4 - lastBit));
        data[4] = (data[4] << left) | (data[3] >> right);
        data[3] = (data[3] << left) | (data[2] >> right);
        data[2] = (data[2] << left) | (data[1] >> right);
        data[1] = (data[1] << left) | (data[0] >> right);
        data[0] = (data[0] << left);
    }

    typedef Protocol<VisonicPacket> P;
    typedef P::Seq<
        P::Array<uint8_t, 5, &VisonicPacket::data>,
        P::Binary<uint8_t, &VisonicPacket::lastBit>,
        P::Binary<bool, &VisonicPacket::flipped>
    > DefaultProtocol;
};

template <typename pulsecounter_t, uint8_t fifoSize = 50>
class VisonicDecoder {
public:
    uint16_t totalBits = 0;
private:
    typedef Logging::Log<Loggers::VisonicDecoder> log;

    typedef typename pulsecounter_t::comparator_t comparator_t;
    typedef typename pulsecounter_t::count_t count_t;

    enum class State { UNKNOWN, PREVIOUS_LONG, PREVIOUS_SHORT };

    State state = State::UNKNOWN;
    bool haveFlipped = false;
    VisonicPacket packet;
    uint8_t bit = 0;
    uint8_t pos = 0;
    Fifo<fifoSize> fifo;

    void writePacket() {
        packet.flipped = haveFlipped;
        log::timeStart();
        fifo.write(&packet);
        log::timeEnd();
    }

    void reset() {
        log::debug("  reset");

        if ((uint16_t(pos * 8) + bit) > totalBits) {
            totalBits = (pos * 8) + bit;
        }

        if (pos >= 4) {
            packet.pad(bit);
            writePacket();
        }

        state = State::UNKNOWN;
        haveFlipped = false;
        bit = 0;
        pos = 0;
        packet.data[0] = 0;
        packet.data[1] = 0;
        packet.data[2] = 0;
        packet.data[3] = 0;
        packet.data[4] = 0;
    }

    void handleBit(uint8_t value) {
        log::debug("  got bit %d of byte %d", bit, pos);

        if (value) {
            packet.data[pos] |= (1 << bit);
        }

        if (bit < 7) {
            bit++;
        } else {
            bit = 0;
            pos++;
        }

        if (pos >= 4 && bit >= 4) {        // 36 bits = 4 1/2 bytes of data in one packet
            log::debug("  *** packet! ");
            reset();
        }
    }

    void flipBits() {
        for (uint8_t i = 0; i < pos; i++) {
            packet.data[i] = ~packet.data[i];
        }
        for (uint8_t i = 0; i < bit; i++) {
            packet.data[pos] ^= (1 << i);
        }
    }

    void handleLongPulse() {
        log::debug("  handling long pulse in state %d", state);
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
        log::debug("  handling short pulse in state %d", state);
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
                log::debug("  two flips!");
                reset();
            } else {
                log::debug("  ***flipping");
                flipBits();
                haveFlipped = true;
                state = State::PREVIOUS_SHORT;
            }
            return;
        }
    }

    static constexpr count_t us_200  = (200_us).template toCounts<comparator_t>().getValue();
    static constexpr count_t us_600  = (600_us).template toCounts<comparator_t>().getValue();
    static constexpr count_t us_1000  = (1000_us).template toCounts<comparator_t>().getValue();

public:
    void apply(const Pulse &pulse) {
        if (pulse.isDefined()) {
            // TODO investigate whether these are actually LOw or HIGH
            log::debug("%d: %d", pulse.isHigh(), pulse.getDuration());
            uint16_t length = pulse.getDuration();
            if (length < us_1000) {
                if (length > us_200) {
                    if (length >= us_600) {
                        handleLongPulse();
                    } else {
                        handleShortPulse();
                    }
                } else {
                    log::debug("  too short");
                    reset();
                }
            } else {
                log::debug("  too long");
                reset();
            }
        } else {
            reset();
        }
    }

    inline ReadResult read(VisonicPacket *p) {
        return fifo.read(p);
    }

};

}

using Visonic::VisonicDecoder;

template <typename pulsecounter_t, uint8_t fifoSize = 50>
VisonicDecoder<pulsecounter_t, fifoSize> visonicDecoder(const pulsecounter_t &counter) {
    return VisonicDecoder<pulsecounter_t, fifoSize>();
}


#endif /* VISONICDECODER_HPP_ */
