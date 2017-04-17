#pragma once

#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Streams/Protobuf.hpp"

namespace HopeRF {

using namespace Time;
using namespace Streams;

struct Ack {
    uint8_t seq;
    uint16_t nodeId;

    typedef Protobuf::Protocol<Ack> P;

    typedef P::Message<
        P::Varint<1, uint16_t, &Ack::nodeId>,
        P::Varint<2, uint8_t, &Ack::seq>
    > DefaultProtocol;
};

}


