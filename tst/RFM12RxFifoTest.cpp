#include <gtest/gtest.h>
#include "RFM12RxFifo.hpp"

TEST(RFM12RxFifoTest, rx_receives_empty_packet_with_correct_crc) {
    RFM12RxFifo<32,true> fifo;

    EXPECT_FALSE(fifo.isWriting());

    fifo.writeStart(0);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0xBF);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0x40);
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_TRUE(fifo.hasContent());

    {
        auto in = fifo.in();
        EXPECT_EQ(0, in.getRemaining());
    }
}

TEST(RFM12RxFifoTest, rx_ignores_empty_packet_with_incorrect_crc) {
    RFM12RxFifo<32,true> fifo;

    EXPECT_FALSE(fifo.isWriting());

    fifo.writeStart(0);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0xBF);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0x41);
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_FALSE(fifo.hasContent());
}

TEST(RFM12RxFifoTest, rx_receives_1_byte_packet_with_correct_crc) {
    RFM12RxFifo<32,true> fifo;

    EXPECT_FALSE(fifo.isWriting());

    fifo.writeStart(1);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0x42);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0x80);
    EXPECT_TRUE(fifo.isWriting());

    fifo.write(0x11);
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_TRUE(fifo.hasContent());

    {
        uint8_t r = 123;
        auto in = fifo.in();
        EXPECT_EQ(1, in.getRemaining());
        in >> r;
        EXPECT_EQ(0x42, r);
        EXPECT_TRUE(bool(in));
    }

    EXPECT_FALSE(fifo.hasContent());
}

TEST(RFM12RxFifoTest, ridiculous_long_packet_does_not_upset_fifo_and_is_rejected) {
    RFM12RxFifo<32,true> fifo;

    fifo.writeStart(254);
    for (int i = 0; i < 63; i++) {
        fifo.write(i);
    }
    fifo.writeAbort();

    EXPECT_FALSE(fifo.isWriting());
    EXPECT_FALSE(fifo.hasContent());
}
