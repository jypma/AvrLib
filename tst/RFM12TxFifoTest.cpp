#include <gtest/gtest.h>
#include "RFM12TxFifo.hpp"

TEST(RFM12TxFifoTest, tx_fifo_wraps_empty_packet_correctly) {
    RFM12TxFifo<> fifo;

    { EXPECT_TRUE((bool) fifo.out()); }

    fifo.readStart();
    EXPECT_TRUE(fifo.isReading());
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x00, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0xBF, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x40, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_FALSE(fifo.isReading());
}

TEST(RFM12TxFifoTest, tx_fifo_wraps_1_byte_packet_correctly) {
    RFM12TxFifo<> fifo;

    const uint8_t byte1 = 0x42;
    { EXPECT_TRUE(fifo.out() << byte1); }

    fifo.readStart();
    EXPECT_TRUE(fifo.isReading());
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(1, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x42, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x80, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0x11, in);
    EXPECT_TRUE(fifo.isReading());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_FALSE(fifo.isReading());
}
