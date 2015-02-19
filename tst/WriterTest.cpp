#include <gtest/gtest.h>
#include "Writer.hpp"

struct MockOut {

    uint8_t buffer[200];
    uint8_t length = 0;
    bool hasStarted = false;
    bool hasEnded = false;

    static void writeStart(void *ctx) {
        ((MockOut*)(ctx))->hasStarted = true;
    }
    static void writeEnd(void *ctx) {
        ((MockOut*)(ctx))->hasEnded = true;
    }
    static bool write(void *ctx, uint8_t b) {
        auto out = ((MockOut*)(ctx));
        out->buffer[out->length] = b;
        out->length++;
        return true;
    }
};

const Writer::VTable vtable = { &MockOut::writeStart, &MockOut::writeEnd, &MockOut::write };

TEST(WriterTest, raw_bytes_are_output_correctly) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        EXPECT_TRUE(out.hasStarted);
        w << uint8_t(42);
        w << uint8_t(84);
        EXPECT_FALSE(out.hasEnded);
    }
    EXPECT_EQ(42, out.buffer[0]);
    EXPECT_EQ(84, out.buffer[1]);
    EXPECT_EQ(2, out.length);
    EXPECT_TRUE(out.hasEnded);
}

TEST(WriterTest, uint16_is_output_msb_first) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << uint16_t(0xFFEE);
    }
    EXPECT_EQ(0xFF, out.buffer[0]);
    EXPECT_EQ(0xEE, out.buffer[1]);
    EXPECT_EQ(2, out.length);
}

TEST(WriterTest, uint32_is_output_msb_first) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << uint32_t(0xFFEEDDCC);
    }
    EXPECT_EQ(0xFF, out.buffer[0]);
    EXPECT_EQ(0xEE, out.buffer[1]);
    EXPECT_EQ(0xDD, out.buffer[2]);
    EXPECT_EQ(0xCC, out.buffer[3]);
    EXPECT_EQ(4, out.length);
}

TEST(WriterTest, null_string_is_ignored) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << (char*)(nullptr);
    }
    EXPECT_EQ(0, out.length);
}

TEST(WriterTest, string_is_outputted) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << "42";
    }
    EXPECT_EQ('4', out.buffer[0]);
    EXPECT_EQ('2', out.buffer[1]);
    EXPECT_EQ(2, out.length);
}

TEST(WriterTest, decimal_uint8_is_handled) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << dec(uint8_t(0));
        w << dec(uint8_t(106));
        w << dec(uint8_t(10));
        w << dec(uint8_t(255));
    }
    EXPECT_EQ('0', out.buffer[0]);
    EXPECT_EQ('1', out.buffer[1]);
    EXPECT_EQ('0', out.buffer[2]);
    EXPECT_EQ('6', out.buffer[3]);
    EXPECT_EQ('1', out.buffer[4]);
    EXPECT_EQ('0', out.buffer[5]);
    EXPECT_EQ('2', out.buffer[6]);
    EXPECT_EQ('5', out.buffer[7]);
    EXPECT_EQ('5', out.buffer[8]);
    EXPECT_EQ(9, out.length);
}

TEST(WriterTest, decimal_int8_is_handled) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << dec(int8_t(0));
        w << dec(int8_t(-1));
        w << dec(int8_t(127));
        w << dec(int8_t(-128));
    }
    EXPECT_EQ('0', out.buffer[0]);
    EXPECT_EQ('-', out.buffer[1]);
    EXPECT_EQ('1', out.buffer[2]);
    EXPECT_EQ('1', out.buffer[3]);
    EXPECT_EQ('2', out.buffer[4]);
    EXPECT_EQ('7', out.buffer[5]);
    EXPECT_EQ('-', out.buffer[6]);
    EXPECT_EQ('1', out.buffer[7]);
    EXPECT_EQ('2', out.buffer[8]);
    EXPECT_EQ('8', out.buffer[9]);
    EXPECT_EQ(10, out.length);
}

TEST(WriterTest, decimal_uint16_t_is_handled) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << dec(uint16_t(0));
        w << dec(uint16_t(65535));
        w << dec(uint16_t(256));
    }
    EXPECT_EQ('0', out.buffer[0]);
    EXPECT_EQ('6', out.buffer[1]);
    EXPECT_EQ('5', out.buffer[2]);
    EXPECT_EQ('5', out.buffer[3]);
    EXPECT_EQ('3', out.buffer[4]);
    EXPECT_EQ('5', out.buffer[5]);
    EXPECT_EQ('2', out.buffer[6]);
    EXPECT_EQ('5', out.buffer[7]);
    EXPECT_EQ('6', out.buffer[8]);
    EXPECT_EQ(9, out.length);
}

TEST(WriterTest, decimal_int16_t_is_handled) {
    MockOut out;
    {
        Writer w(&vtable, &out);
        w << dec(int16_t(0));
        w << dec(int16_t(32767));
        w << dec(int16_t(-32678));
    }
    EXPECT_EQ('0', out.buffer[0]);
    EXPECT_EQ('3', out.buffer[1]);
    EXPECT_EQ('2', out.buffer[2]);
    EXPECT_EQ('7', out.buffer[3]);
    EXPECT_EQ('6', out.buffer[4]);
    EXPECT_EQ('7', out.buffer[5]);
    EXPECT_EQ('-', out.buffer[6]);
    EXPECT_EQ('3', out.buffer[7]);
    EXPECT_EQ('2', out.buffer[8]);
    EXPECT_EQ('6', out.buffer[9]);
    EXPECT_EQ('7', out.buffer[10]);
    EXPECT_EQ('8', out.buffer[11]);
    EXPECT_EQ(12, out.length);
}
