#include <gtest/gtest.h>
#include "Reader.hpp"

struct MockIn {
    uint8_t buffer[200];
    uint8_t remaining;
    uint8_t pos = 0;
    bool hasStarted = false;
    bool hasEnded = false;

    static void readStart(void * ctx) {
        ((MockIn*)ctx)->hasStarted = true;
    }
    static void readEnd(void *ctx) {
        ((MockIn*)ctx)->hasEnded = true;
    }
    static bool read(void *ctx, uint8_t &target) {
        MockIn *in = ((MockIn*)ctx);
        target = in->buffer[in->pos];
        in->pos++;
        in->remaining--;
        return true;
    }
    static uint8_t getRemaining(void *ctx) {
        return ((MockIn*)ctx)->remaining;
    }
};

const Reader::VTable vtable = { &MockIn::readStart, &MockIn::readEnd, &MockIn::read, &MockIn::getRemaining };

enum class ReaderTestEnum: uint8_t {ONE, TWO};

TEST(ReaderTest, raw_bytes_and_enums_are_read_correctly) {
    MockIn in;
    in.buffer[0] = 42;
    in.buffer[1] = 1;
    in.remaining = 2;
    {
        Reader r(&vtable, &in);
        EXPECT_TRUE(in.hasStarted);
        EXPECT_EQ(2, r.getRemaining());
        uint8_t inbyte;
        r >> inbyte;
        EXPECT_EQ(42, inbyte);
        ReaderTestEnum inenum;
        r >> inenum;
        EXPECT_EQ(ReaderTestEnum::TWO, inenum);
        EXPECT_FALSE(in.hasEnded);
    }
    EXPECT_TRUE(in.hasEnded);
}
