#include <gtest/gtest.h>
#include "HopeRF/JeeLibTxFifo.hpp"

using namespace HopeRF;

struct JeeLibTxFifoCallbackTest {
    bool invoked = false;
    static void onWriteEnd(JeeLibTxFifoCallbackTest &target) {
        target.invoked = true;
    }
};

TEST(RFM12JeeLibTxFifo, standard_packet_is_encoded_with_preamble_crc_and_postfix) {
    JeeLibTxFifoCallbackTest cb;
    JeeLibTxFifo<JeeLibTxFifoCallbackTest,JeeLibTxFifoCallbackTest> fifo(cb);

    fifo.write_fsk(30, uint8_t(82), uint8_t(49), uint8_t(32), uint8_t(32), uint8_t(1), uint8_t(58), uint8_t(251), uint8_t(0));
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_TRUE(fifo.readStart());

    uint8_t b;
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0x2D, b);
    fifo.read(b);
    EXPECT_EQ(0x05, b); // groupId
    fifo.read(b);
    EXPECT_EQ(30, b); // header
    fifo.read(b);
    EXPECT_EQ(8, b); // length
    fifo.read(b);
    EXPECT_EQ(82, b);
    fifo.read(b);
    EXPECT_EQ(49, b);
    fifo.read(b);
    EXPECT_EQ(32, b);
    fifo.read(b);
    EXPECT_EQ(32, b);
    fifo.read(b);
    EXPECT_EQ(1, b);
    fifo.read(b);
    EXPECT_EQ(58, b);
    fifo.read(b);
    EXPECT_EQ(251, b);
    fifo.read(b);
    EXPECT_EQ(0, b);
    fifo.read(b);
    EXPECT_EQ(80, b); // CRC1
    fifo.read(b);
    EXPECT_EQ(200, b); // CRC2
    fifo.read(b);
    EXPECT_EQ(0xAA, b); // postfix
    fifo.read(b);
    EXPECT_EQ(0xAA, b); // postfix
}

TEST(RFM12JeeLibTxFifo, empty_packet_is_encoded_with_preamble_crc_and_postfix) {
    JeeLibTxFifoCallbackTest cb;
    JeeLibTxFifo<JeeLibTxFifoCallbackTest,JeeLibTxFifoCallbackTest> fifo(cb);

    fifo.write_fsk(0);
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_TRUE(fifo.readStart());

    uint8_t b;
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0xAA, b);
    fifo.read(b);
    EXPECT_EQ(0x2D, b);
    fifo.read(b);
    EXPECT_EQ(5, b); // groupId
    fifo.read(b);
    EXPECT_EQ(0, b); // header
    fifo.read(b);
    EXPECT_EQ(0, b); // length
    fifo.read(b);
    EXPECT_EQ(97, b); // CRC1
    fifo.read(b);
    EXPECT_EQ(193, b); // CRC2
    fifo.read(b);
    EXPECT_EQ(0xAA, b); // postfix
    fifo.read(b);
    EXPECT_EQ(0xAA, b); // postfix
}
