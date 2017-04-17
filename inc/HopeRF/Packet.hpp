#pragma once

#include "Streams/Protobuf.hpp"

namespace HopeRF {

using namespace Streams;

template <typename T>
struct Packet {
    uint8_t seq;
    uint16_t nodeId;
    T body;

    typedef typename Protobuf::Protocol<Packet> P;

    typedef typename P::template Message<
        typename P::template Varint<1, uint16_t, &Packet::nodeId>,
        typename P::template Varint<2, uint8_t, &Packet::seq>,
        typename P::template SubMessage<3, T, &Packet::body>
    > DefaultProtocol;
};

}


