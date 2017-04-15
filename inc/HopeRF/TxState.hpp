#pragma once

#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Ack.hpp"
#include "Packet.hpp"
#include "RFM12.hpp"

namespace HopeRF {

using namespace Time;

template <typename rfm_t, typename rt_t, typename T>
class TxState {
    rfm_t *const rfm;
    rt_t *const rt;
    const uint16_t nodeId;

    T state;
    uint8_t seq = 0;
    bool tx = false;
    VariableDeadline<rt_t> resend = { *rt };
    uint8_t resendCount = 1;

    void send() {
        Packet<T> packet = { seq, nodeId, state };
        rfm->write_fsk(Headers::TXSTATE, &packet);
        tx = true;
        resend.schedule(10_ms * resendCount);
    }
public:
    TxState(rfm_t &r, rt_t &t, T initial, uint16_t _nodeId): rfm(&r), rt(&t), state(initial), nodeId(_nodeId) {
        send();
    }

    void setState(T t) {
        if (t != state) {
            seq++;
            state = t;
            resendCount = 1;
            send();
        }
    }

    void loop() {
        if (rfm->in().hasContent()) {
            uint8_t header;
            Ack message;
            if (rfm->in().read(&header, &message)) {
                if (header == Headers::ACK && message.nodeId == nodeId && message.seq == seq) {
                    tx = false;
                    resend.cancel();
                    resendCount = 1;
                }
            }
        } else if (tx && resend.isNow()) {
            if (resendCount < 200) { // resend at least once every 200ms
                resendCount++;
            }
            send();
        }
    }
};

}


