#pragma once

#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Streams/Protobuf.hpp"
#include "RFM12.hpp"

namespace HopeRF {

using namespace Time;
using namespace Streams;

struct Request {
    uint16_t nodeId;

    typedef Protobuf::Protocol<Request> P;

    typedef P::Message<
        P::Varint<1, uint16_t, &Request::nodeId>
    > DefaultProtocol;
};

template <typename in_t>
bool readRequest(in_t in, uint16_t nodeId) {
    if (!in.hasContent()) {
        return false;
    }

    Request msg;
    in.readStart();
    if (in.read(FB(Headers::REQ), &msg)) {
        if (msg.nodeId == nodeId) {
            in.readEnd();
            return true;
        }
    }
    in.readAbort();
    return false;
}

}


