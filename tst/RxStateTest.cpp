#include "HopeRF/RxState.hpp"
#include <gtest/gtest.h>
#include "Mocks.hpp"
#include "Streams/Protobuf.hpp"

namespace RxStateTest {

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

TEST(RxStateTest, should_update_state_and_send_ack_when_receiving_valid_packet) {
    MockRFM12 rfm;
    State state = { 42 };
    RxState<MockRFM12, State> rxState = { rfm, 15, state };
    rfm.recv.write(FB(
        2,              // RFM header
        1 << 3, 15,     // field 1 = 15
        2 << 3, 1,      // field 2 = 1
        3 << 3 | 2, 2,  // field 3, nested, length 2
          1 << 3, 123));//   field 1 = 123

    EXPECT_TRUE(rxState.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_EQ(123, rxState.getState().value);

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        1,             // rfm header (ACK)
        1 << 3, 15,    // field 1 (nodeId)
        2 << 3, 1      // field 2 (seq)
    )));

    EXPECT_FALSE(rxState.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_EQ(123, rxState.getState().value);
}

TEST(RxStateTest, should_send_ack_when_re_receiving_the_same_state) {
    MockRFM12 rfm;
    State state = { 42 };
    RxState<MockRFM12, State> rxState = { rfm, 15, state };

    auto incoming = FB(
            2,              // RFM header
            1 << 3, 15,     // field 1 = 15
            2 << 3, 1,      // field 2 = 1
            3 << 3 | 2, 2,  // field 3, nested, length 2
              1 << 3, 123);

    rfm.recv.write(incoming);
    rxState.isStateChanged();
    rfm.sendFsk.clear();

    rfm.recv.write(incoming);
    ASSERT_FALSE(rxState.isStateChanged());

    EXPECT_TRUE(rfm.sendFsk.read(FB(
        1,             // rfm header (ACK)
        1 << 3, 15,    // field 1 (nodeId)
        2 << 3, 1      // field 2 (seq)
    )));
}

TEST(RxStateTest, should_ignore_invalid_packets) {
    MockRFM12 rfm;
    State state = { 42 };
    RxState<MockRFM12, State> rxState = { rfm, 15, state };

    rfm.recv.write(FB(
        2,              // RFM header
        1 << 3, 16,     // field 1 = 16 (different nodeId)
        2 << 3, 1,      // field 2 = 1
        3 << 3 | 2, 2,  // field 3, nested, length 2
          1 << 3, 123));//   field 1 = 123
    EXPECT_FALSE(rxState.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());

    rfm.recv.write(FB(1, 1 << 3, 15, 2 << 3, 1, 3 << 3 | 2, 2, 1 << 3, 123)); // wrong header
    EXPECT_FALSE(rxState.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());

    rfm.recv.write(FB(2, 5 << 3, 15, 2 << 3, 1, 3 << 3 | 2, 2, 1 << 3, 123)); // invalid protobuf fields
    EXPECT_FALSE(rxState.isStateChanged());
    EXPECT_FALSE(rfm.recv.isReading());
    EXPECT_FALSE(rfm.recv.isEmpty());
}

}
