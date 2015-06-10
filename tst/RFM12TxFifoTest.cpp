#include <gtest/gtest.h>
#include "HopeRF/RFM12TxFifo.hpp"

using namespace HopeRF;

struct MockOOKType {
    int foo;
};

TEST(RFM12TxFifoTest, tx_fifo_wraps_empty_packet_correctly) {
    RFM12TxFifo<> fifo;

    { EXPECT_TRUE((bool) fifo.out(nullptr)); }

    EXPECT_TRUE(fifo.readStart());
    EXPECT_TRUE(fifo.hasReadAvailable());
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x00, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xBF, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x40, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_FALSE(fifo.hasReadAvailable());
}

TEST(RFM12TxFifoTest, tx_fifo_wraps_1_byte_packet_correctly) {
    RFM12TxFifo<> fifo;

    const uint8_t byte1 = 0x42;
    { EXPECT_TRUE(fifo.out(nullptr) << byte1); }

    fifo.readStart();
    EXPECT_TRUE(fifo.hasReadAvailable());
    uint8_t in;
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x2D, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(1, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x42, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x80, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0x11, in);
    EXPECT_TRUE(fifo.hasReadAvailable());
    fifo.read(in);
    EXPECT_EQ(0xAA, in);
    EXPECT_FALSE(fifo.hasReadAvailable());
}

TEST(RFM12TxFifoTest, tx_fifo_returns_false_for_serial_packets_and_aborts_its_own_read) {
    RFM12TxFifo<> fifo;
    SerialConfig config;

    EXPECT_TRUE(fifo.out(&config) << uint8_t(42));

    EXPECT_FALSE(fifo.readStart());

    EXPECT_FALSE(fifo.getChunkedFifo().isReading());
    EXPECT_TRUE(fifo.hasContent());
}

