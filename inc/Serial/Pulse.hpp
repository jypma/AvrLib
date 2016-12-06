/*
 * Pulse.hpp
 *
 *  Created on: Aug 31, 2015
 *      Author: jan
 */

#ifndef SERIAL_PULSE_HPP_
#define SERIAL_PULSE_HPP_

#include "Streams/Format.hpp"
#include "Time/Units.hpp"

namespace Serial {

using namespace Streams;
using namespace Time;

class Pulse {
    bool high;
    uint16_t duration;
public:
    constexpr Pulse(): high(false), duration(0) {}
    constexpr Pulse(const bool _high, const uint16_t _duration): high(_high), duration(_duration) {}

    /**
     * Returns the pulse's duration, in timer counts.
     */
    constexpr uint16_t getDuration() const {
        return duration;
    }
    bool isHigh() const {
        return high;
    }
    bool isLow() const {
        return !high;
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

    typedef Protocol<Pulse> P;
    typedef P::Seq<
        P::Binary<bool, &Pulse::high>,
        P::Binary<uint16_t, &Pulse::duration>
    > DefaultProtocol;

    typedef P::Seq<
        STR("h="),
        P::Decimal<bool, &Pulse::high>,
        STR(",d="),
        P::Decimal<uint16_t, &Pulse::duration>
    > TEXT;
};

template <typename prescaled_t>
class PulseOn: public Pulse {
public:
    using Pulse::Pulse;

    template <typename duration_t>
    constexpr bool operator > (const duration_t duration) const {
        return getDuration() > uint16_t(toCountsOn<prescaled_t>(duration).getValue());
    }

    template <typename duration_t>
    constexpr bool operator < (const duration_t duration) const {
        return getDuration() < uint16_t(toCountsOn<prescaled_t>(duration).getValue());
    }

    template <typename duration_t>
    constexpr bool operator >= (const duration_t duration) const {
        return getDuration() >= toCountsOn<prescaled_t>(duration);
    }

    template <typename duration_t>
    constexpr bool operator <= (const duration_t duration) const {
        return getDuration() <= toCountsOn<prescaled_t>(duration);
    }
};

template <typename prescaled_t, typename Value>
PulseOn<prescaled_t> constexpr highPulseOn(Value duration) {
    constexpr typename prescaled_t::value_t value = toCountsOn<prescaled_t>(duration).getValue();
    return { true, value };
}

template <typename prescaled_t, typename Value>
PulseOn<prescaled_t> constexpr lowPulseOn(Value duration) {
    constexpr typename prescaled_t::value_t value = toCountsOn<prescaled_t>(duration).getValue();
    return { false, value };
}

}

#endif /* SERIAL_PULSE_HPP_ */
