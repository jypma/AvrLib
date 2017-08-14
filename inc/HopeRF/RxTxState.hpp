#pragma once

#include "TxState.hpp"
#include "RxState.hpp"

namespace HopeRF {

using namespace Time;
using namespace Streams;

/**
 * Receives and sends state packets with a predefined format, automatically sending an ack() when receiving
 * such a packet, and notifying the app when this state has changed.
 *
 * This variant allows both the embedded software to initiate state changes, as well as being able to receive them
 * (local changes take precedence).
 */
template <typename rfm_t, typename rt_t, typename T>
class RxTxState {
    typedef Logging::Log<Loggers::RxState> log;

    rfm_t *const rfm;
    rt_t *const rt;
    const uint16_t nodeId;
    const uint8_t resendOffset; // Fixed offset to add to resend delay, depending on nodeId, to combat interference from several nodes

    T state;
    uint8_t seq = 0;
    bool tx = false;
    VariableDeadline<rt_t> resend = { *rt };
    uint8_t resendCount = 0;

    void send() {
        Packet<T> packet = { seq, nodeId, state };
        rfm->write_fsk(Headers::TXSTATE, &packet);
        tx = true;
        resend.schedule(10_ms * uint8_t(ResendDelays::charAt(resendCount) + resendOffset));
    }
public:
    RxTxState(rfm_t &r, rt_t &t, T initial, uint16_t _nodeId):
        rfm(&r), rt(&t), nodeId(_nodeId),
        resendOffset(((_nodeId) ^ (_nodeId >> 4) ^ (_nodeId >> 8) ^ (_nodeId >> 12)) & 0x000F),
        state(initial) {
        send();
    }

    void setState(T t) {
        if (t != state) {
            seq++;
            state = t;
            resendCount = 0;
            send();
        }
    }

    T getState() const {
        return state;
    }

    bool isStateChanged() {
        if (readAck(rfm->in(), seq, nodeId)) {
            tx = false;
            resend.cancel();
            resendCount = 0;
        } else if (tx && resend.isNow()) {
            if (resendCount < ResendDelays::size() - 1) {
                resendCount++;
            }
            send();
        }
        for (auto packet: readPacket<T>(rfm->in(), nodeId)) {
            if (packet.seq == seq + 1 || (packet.seq == seq && !(packet.body != state))) {
                const Ack ack = { packet.seq, nodeId };
                rfm->write_fsk(Headers::ACK, &ack);
                rfm->write_fsk(Headers::ACK, &ack);

                seq = packet.seq;
                if (packet.body != state) {
                    state = packet.body;
                    return true;
                } else {
                    return false;
                }
            } else {
                log::debug(F("Invalid seqnr. Got "), dec(packet.seq), F(" expected "), dec(seq + 1));
                // TODO send nack
                return false;
            }
        }
        return false;
    }
};

}
