#include "HopeRF/TxState.hpp"
#include <gtest/gtest.h>
#include "Mocks.hpp"
#include "Streams/Protobuf.hpp"

namespace TxStateTest {

using namespace Mocks;
using namespace HopeRF;
using namespace Streams;

struct State {
    uint8_t value;

    typedef Protobuf::Protocol<State> P;

    typedef P::Message<
        P::Varint<1, uint8_t, &State::value>
    > DefaultProtocol;

    bool operator!= (const State &b) { return value != b.value; }
};

TEST(TxStateTest, should_transmit_on_creation) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };
    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST(TxStateTest, should_transmit_new_state_even_if_previous_was_not_acked) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };

    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };
    rfm.sendFsk.clear();
    state.value = 84;
    txState.setState(state);

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 1,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 84  //   field 1 (value)
        )));
}

TEST(TxStateTest, should_retransmit_when_unacked_and_timer_fires) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };

    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };
    rfm.sendFsk.clear();
    rt.advance(130_ms); // resend offset is 12*10ms, plus extra 10ms for first delay.

    txState.loop();
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST(TxStateTest, should_not_retransmit_if_acked_when_timer_fires) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };

    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0));   // field 2 (seq)
    txState.loop();
    EXPECT_TRUE(rfm.sendFsk.isEmpty());

    rt.advance(130_ms);
    EXPECT_TRUE(rfm.sendFsk.isEmpty());
}

TEST(TxStateTest, should_retransmit_when_acked_on_wrong_sequence_number) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };

    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 1));   // field 2 (seq, doesn't match what was transmitted (=0))
    txState.loop();
    rt.advance(130_ms);
    txState.loop();

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST(TxStateTest, should_retransmit_when_acked_on_wrong_node_id) {
    MockRFM12 rfm;
    MockRealTimer rt;
    State state = { 42 };

    TxState<MockRFM12, MockRealTimer, State> txState = { rfm, rt, state, 123 };
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 124,   // field 1 (nodeId, wrong)
        2 << 3, 0));   // field 2 (seq)
    txState.loop();
    rt.advance(130_ms);
    txState.loop();

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

}
