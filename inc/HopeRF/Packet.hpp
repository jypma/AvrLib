#pragma once

#include "Streams/Protobuf.hpp"
#include "Option.hpp"
#include "RFM12.hpp"

namespace HopeRF {

using namespace Streams;

template <typename T>
struct Packet {
    uint8_t seq;
    uint16_t nodeId;
    T body;

    constexpr bool operator!=(const Packet<T> &that) const {
        return seq != that.seq || nodeId != that.nodeId || body != that.body;
    }

    typedef typename Protobuf::Protocol<Packet> P;

    typedef typename P::template Message<
        typename P::template Varint<1, uint16_t, &Packet::nodeId>,
        typename P::template Varint<2, uint8_t, &Packet::seq>,
        typename P::template SubMessage<3, T, &Packet::body>
    > DefaultProtocol;
};

template <typename T, typename in_t>
Option<Packet<T>> readPacket(in_t in, const uint16_t nodeId) {
  typedef Logging::Log<Loggers::RxState> log;

    if (!in.hasContent()) {
        return none();
    }
    in.readStart();
    Packet<T> packet;
    if (in.read(FB(Headers::RXSTATE), &packet)) {
        if (packet.nodeId == nodeId) {
            in.readEnd();
            return packet;
        } else {
          log::debug(F("Ign "), dec(packet.nodeId), '(', char(packet.nodeId << 8), char(packet.nodeId & 0xFF), ')');
          log::debug(F("Exp "), dec(nodeId), '(', char(packet.nodeId << 8), char(packet.nodeId & 0xFF), ')');
        }

    }
    in.readAbort();
    return none();
}

}


