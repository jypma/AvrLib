#include <gtest/gtest.h>
#include <type_traits>
#include "HopeRF/JeeLibRxFifo.hpp"

using namespace HopeRF;

TEST(RFM12JeeLibRxFifo, rx_receives_packet_with_valid_crc) {
    JeeLibRxFifo<5,32,true> fifo;

    uint8_t bytes[] = {30,8,82,49,32,32,1,58,251,0,80,200};

    EXPECT_FALSE(fifo.isWriting());
    fifo.writeStart(bytes[0]);
    EXPECT_TRUE(fifo.isWriting());
    for (unsigned int i = 1; i < std::extent<decltype(bytes)>::value; i++) {
        fifo.write(bytes[i]);
    }
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_TRUE(fifo.in().hasContent());
    fifo.in().readStart();
    EXPECT_EQ(9, fifo.in().getReadAvailable()); // 1 byte header + 8 bytes in the original packet.
    uint8_t b;
    fifo.in().read(&b);
    EXPECT_EQ(bytes[0], b); // header
    for (unsigned int i = 2; i < std::extent<decltype(bytes)>::value - 2; i++) {
        fifo.in().read(&b);
        EXPECT_EQ(bytes[i], b);
    }
}

TEST(RFM12JeeLibRxFifo, rx_receives_empty_packet_with_valid_crc) {
    JeeLibRxFifo<5,32,true> fifo;

    uint8_t bytes[] = {0,0,97,193};

    EXPECT_FALSE(fifo.isWriting());
    fifo.writeStart(bytes[0]);
    EXPECT_TRUE(fifo.isWriting());
    for (unsigned int i = 1; i < std::extent<decltype(bytes)>::value; i++) {
        fifo.write(bytes[i]);
    }
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_TRUE(fifo.in().hasContent());
    fifo.in().readStart();
    EXPECT_EQ(1, fifo.in().getReadAvailable()); // 1 byte header + 0 bytes in the original packet.
    uint8_t b;
    fifo.in().read(&b);
    EXPECT_EQ(bytes[0], b); // header
    for (unsigned int i = 2; i < std::extent<decltype(bytes)>::value - 2; i++) {
        fifo.in().read(&b);
        EXPECT_EQ(bytes[i], b);
    }
}

TEST(RFM12JeeLibRxFifo, rx_does_not_choke_on_value_254_for_length) {
    JeeLibRxFifo<5,32,true> fifo;
    fifo.writeStart(31); // header;
    fifo.write(254); // length, which is just noise. Internally capped at 64 + 2 = 66.
    for (int i = 0; i < 66; i++) fifo.write(0);
    // by now, the packet should have been read (and ignored since the CRC won't match)
    EXPECT_FALSE(fifo.isWriting());
}
