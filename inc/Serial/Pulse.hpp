/*
 * Pulse.hpp
 *
 *  Created on: Aug 31, 2015
 *      Author: jan
 */

#ifndef SERIAL_PULSE_HPP_
#define SERIAL_PULSE_HPP_

#include "Streams/Streamable.hpp"
#include "Time/Units.hpp"

namespace Serial {

using namespace Streams;
using namespace Time;

class Pulse: public Streamable<Pulse> {
    bool high;
    uint16_t duration;
public:
    constexpr Pulse(): high(false), duration(0) {}
    constexpr Pulse(const bool _high, const uint16_t _duration): high(_high), duration(_duration) {}

    uint16_t getDuration() const {
        return duration;
    }
    bool isHigh() const {
        return high;
    }
    bool isEmpty() const {
        return duration == 0;
    }
    bool isDefined() const {
        return duration != 0;
    }

    static constexpr Pulse empty() {
        return Pulse();
    }

    typedef Format<
        Scalar<bool, &Pulse::high>,
        Scalar<uint16_t, &Pulse::duration>
    > Proto;
};

template <typename prescaled_t, typename Value>
Pulse constexpr highPulseOn(Value duration) {
    return Pulse(true, toCountsOn<prescaled_t>(duration));
}

template <typename prescaled_t, typename Value>
Pulse constexpr lowPulseOn(Value duration) {
    return Pulse(false, toCountsOn<prescaled_t>(duration));
}

}

#endif /* SERIAL_PULSE_HPP_ */
