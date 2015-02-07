#include <gtest/gtest.h>
#include "RFM12TxFifo.hpp"

TEST(RFM12TxFifoTest, tx_fifo_wraps_empty_packet_correctly) {
    Fifo<16> data;
    Fifo<4> lengths;
    ChunkedFifo f(&data, &lengths);
    RFM12TxFifo fifo(&f);

    { EXPECT_TRUE((bool) fifo.out()); }

    fifo.readStart();
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    fifo.read(in);
    EXPECT_EQ(0x00, in);
    fifo.read(in);
    EXPECT_EQ(0xBF, in);
    fifo.read(in);
    EXPECT_EQ(0x40, in);
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    in = 123;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
}

TEST(RFM12TxFifoTest, tx_fifo_wraps_1_byte_packet_correctly) {
    Fifo<16> data;
    Fifo<4> lengths;
    ChunkedFifo f(&data, &lengths);
    RFM12TxFifo fifo(&f);

    const uint8_t byte1 = 0x42;
    { EXPECT_TRUE(fifo.out() << byte1); }

    fifo.readStart();
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    fifo.read(in);
    EXPECT_EQ(1, in);
    fifo.read(in);
    EXPECT_EQ(0x42, in);
    fifo.read(in);
    EXPECT_EQ(0x80, in);
    fifo.read(in);
    EXPECT_EQ(0x11, in);
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    in = 123;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
}
