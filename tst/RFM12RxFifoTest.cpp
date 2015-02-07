#include <gtest/gtest.h>
#include "RFM12RxFifo.hpp"

TEST(RFM12RxFifoTest, rx_receives_empty_packet_with_correct_crc) {
    Fifo<16> data;
    Fifo<4> lengths;
    ChunkedFifo f(&data, &lengths);
    RFM12RxFifo fifo(&f);

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
    Fifo<16> data;
    Fifo<4> lengths;
    ChunkedFifo f(&data, &lengths);
    RFM12RxFifo fifo(&f);

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
    Fifo<16> data;
    Fifo<4> lengths;
    ChunkedFifo f(&data, &lengths);
    RFM12RxFifo fifo(&f);

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
