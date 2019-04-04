#pragma once

#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Streams/Protobuf.hpp"
#include "RFM12.hpp"

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

template <typename in_t>
bool readAck(in_t in, uint8_t seq, uint16_t nodeId) {
    if (!in.hasContent()) {
        return false;
    }

    Ack ack;
    in.readStart();
    if (in.read(FB(Headers::RX_ACK), &ack)) {
        if (ack.nodeId == nodeId && ack.seq == seq) {
            in.readEnd();
            return true;
        }
    }
    in.readAbort();
    return false;
}

}


