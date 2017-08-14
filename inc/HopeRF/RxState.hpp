#pragma once

#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Streams/Protobuf.hpp"
#include "Ack.hpp"
#include "Packet.hpp"
#include "RFM12.hpp"
#include "Logging.hpp"

namespace HopeRF {

using namespace Time;
using namespace Streams;

/**
 * Receives state packets with a predefined format, automatically sending an ack() when receiving
 * such a packet, and notifying the app when this state has changed.
 */
template <typename rfm_t, typename T>
class RxState {
    typedef Logging::Log<Loggers::RxState> log;

    rfm_t * const rfm;
    const uint16_t nodeId;

    T state;
public:
    RxState(rfm_t &_rfm, uint16_t _nodeId, T initial):
        rfm(&_rfm), nodeId(_nodeId), state(initial) {}

    bool isStateChanged() {
        for (auto packet: readPacket<T>(rfm->in(), nodeId)) {
            const Ack ack = { packet.seq, nodeId };
            rfm->write_fsk(Headers::ACK, &ack);
            rfm->write_fsk(Headers::ACK, &ack);

            if (packet.body != state) {
                state = packet.body;
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    T getState() const {
        return state;
    }
};

}


