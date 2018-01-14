#include "HopeRF/RxTxState.hpp"
#include <gtest/gtest.h>
#include "Mocks.hpp"
#include "Streams/Protobuf.hpp"

namespace RxTxStateTest {

using namespace Mocks;
using namespace HopeRF;
using namespace Streams;

struct State {
    uint8_t value;

    typedef Protobuf::Protocol<State> P;

    typedef P::Message<
        P::Varint<1, uint8_t, &State::value>
    > DefaultProtocol;

    bool operator!= (const State &b) const { return value != b.value; }
};

struct RxTxStateTest : public ::testing::Test {
    MockRFM12 rfm;
    MockRealTimer rt;
    RxTxState<MockRFM12, MockRealTimer, State> state = { rfm, rt, { 42 }, 123 };
};

TEST_F(RxTxStateTest, should_update_state_and_send_ack_when_receiving_valid_packet) {
    rfm.sendFsk.clear();

    rfm.recv.write(FB(
        2,              // RFM header
        1 << 3, 123,     // field 1 = 123
        2 << 3, 1,      // field 2 = 1
        3 << 3 | 2, 2,  // field 3, nested, length 2
          1 << 3, 123));//   field 1 = 123

    EXPECT_TRUE(state.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_EQ(123, state.get().value);

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        1,             // rfm header (ACK)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 1      // field 2 (seq)
    )));

    EXPECT_FALSE(state.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_EQ(123, state.get().value);
}

TEST_F(RxTxStateTest, should_send_ack_when_re_receiving_the_same_state) {
    rfm.sendFsk.clear();

    auto incoming = FB(
            2,              // RFM header
            1 << 3, 123,     // field 1 = 123
            2 << 3, 1,      // field 2 = 1
            3 << 3 | 2, 2,  // field 3, nested, length 2
              1 << 3, 42);

    rfm.recv.write(incoming);
    state.isStateChanged();
    rfm.sendFsk.clear();

    rfm.recv.write(incoming);
    ASSERT_FALSE(state.isStateChanged());

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        1,             // rfm header (ACK)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 1      // field 2 (seq)
    )));
}

TEST_F(RxTxStateTest, should_ignore_invalid_packets) {
    rfm.sendFsk.clear();

    rfm.recv.write(FB(
        2,              // RFM header
        1 << 3, 16,     // field 1 = 16 (different nodeId)
        2 << 3, 1,      // field 2 = 1
        3 << 3 | 2, 2,  // field 3, nested, length 2
          1 << 3, 123));//   field 1 = 123
    EXPECT_FALSE(state.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());

    rfm.recv.write(FB(1, 1 << 3, 123, 2 << 3, 1, 3 << 3 | 2, 2, 1 << 3, 123)); // wrong header
    EXPECT_FALSE(state.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());

    rfm.recv.write(FB(2, 5 << 3, 123, 2 << 3, 1, 3 << 3 | 2, 2, 1 << 3, 123)); // invalid protobuf fields
    EXPECT_FALSE(state.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());
}

TEST_F(RxTxStateTest, should_transmit_on_creation) {
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_transmit_new_state_even_if_previous_was_not_acked) {
    rfm.sendFsk.clear();
    state.set({ 84 });

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 1,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 84  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_retransmit_when_unacked_and_timer_fires) {
    rfm.sendFsk.clear();
    rt.advance(130_ms); // resend offset is 12*10ms, plus extra 10ms for first delay.

    state.isStateChanged();
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_not_retransmit_if_acked_when_timer_fires) {
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 0));   // field 2 (seq)
    state.isStateChanged();
    EXPECT_TRUE(rfm.sendFsk.isEmpty());

    rt.advance(130_ms);
    EXPECT_TRUE(rfm.sendFsk.isEmpty());
}

TEST_F(RxTxStateTest, should_retransmit_when_acked_on_wrong_sequence_number) {
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 1));   // field 2 (seq, doesn't match what was transmitted (=0))
    state.isStateChanged();
    rt.advance(130_ms);
    state.isStateChanged();

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_retransmit_when_acked_on_wrong_node_id) {
    rfm.sendFsk.clear();
    rfm.recv.write(FB(
        1,             // rfm header (ack)
        1 << 3, 124,   // field 1 (nodeId, wrong)
        2 << 3, 0));   // field 2 (seq)
    state.isStateChanged();
    rt.advance(130_ms);
    state.isStateChanged();

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header (txState)
        1 << 3, 123,    // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_increase_retransmit_delay_until_maximum_and_then_stop_retransmitting) {
    auto maxDelay = 1560_ms;  // (12+144) * 10ms
    for (int i = 0; i < 20; i++) {
        rfm.sendFsk.clear();
        rt.advance(maxDelay);
        state.isStateChanged();
    }

    rfm.sendFsk.clear();
    rt.advance(130_ms);  // not yet, by now we should have reached max delay.
    state.isStateChanged();
    EXPECT_TRUE(rfm.sendFsk.isEmpty());

    rt.advance(maxDelay);
    state.isStateChanged();
    EXPECT_TRUE(rfm.sendFsk.isEmpty());
}

TEST_F(RxTxStateTest, should_send_request_packet_when_request_is_invoked_and_accept_any_seqnr_after_that) {
    rfm.sendFsk.clear();

    state.requestLatest();
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        4,             // rfm header (request)
        1 << 3, 123    // field 1 (nodeId)
    )));

    rfm.sendFsk.clear();
    auto incoming = FB(
            2,              // RFM header
            1 << 3, 123,    // field 1, nodeId = 123
            2 << 3, 15,      // field 2, seq = 15
            3 << 3 | 2, 2,  // field 3, nested, length 2
              1 << 3, 123);
    rfm.recv.write(incoming);
    EXPECT_TRUE(state.isStateChanged());
    EXPECT_EQ(123, state.get().value);
}

TEST_F(RxTxStateTest, should_resend_state_when_receiving_request) {
    rfm.sendFsk.clear();

    rfm.recv.write(FB(
        4,            // rfm header (request)
        1 << 3, 123   // field 1 (nodeId)
    ));
    EXPECT_FALSE(state.isStateChanged());
    EXPECT_EQ(42, state.get().value);
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

TEST_F(RxTxStateTest, should_send_current_state_when_receiving_invalid_seqnr) {
    rfm.sendFsk.clear();

    rfm.recv.write(FB(
        2,              // RFM header
        1 << 3, 123,    // field 1 = 123
        2 << 3, 5,      // field 2 = 5 (invalid seqnr)
        3 << 3 | 2, 2,  // field 3, nested, length 2
          1 << 3, 123));//   field 1 = 123

    EXPECT_FALSE(state.isStateChanged());
    EXPECT_EQ(42, state.get().value);
    EXPECT_TRUE(rfm.sendFsk.read(FB(
        3,             // rfm header
        1 << 3, 123,   // field 1 (nodeId)
        2 << 3, 0,     // field 2 (seq)
        3 << 3 | 2, 2, // field 3 (message, length 2)
           1 << 3, 42  //   field 1 (value)
        )));
}

}
