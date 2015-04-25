#include <gtest/gtest.h>
#include "Reader.hpp"

struct MockIn {
    uint8_t buffer[200];
    uint8_t remaining;
    uint8_t pos = 0;
    bool hasStarted = false;
    bool hasCommit = false;
    bool hasRollback = false;
    bool reading = false;

    static void readStart(void * ctx) {
        ((MockIn*)ctx)->hasStarted = true;
    }
    static void readCommit(void *ctx) {
        ((MockIn*)ctx)->hasCommit = true;
    }
    static void readRollback(void *ctx) {
        ((MockIn*)ctx)->hasRollback = true;
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
    static bool isReading(void *ctx) {
        return ((MockIn*)ctx)->reading;
    }
};

const Reader::VTable vtable = { &MockIn::readStart, &MockIn::readCommit, &MockIn::readRollback, &MockIn::read, &MockIn::getRemaining, &MockIn::isReading };

enum class ReaderTestEnum: uint8_t {ONE, TWO};

TEST(ReaderTest, does_not_commit_if_constructed_on_input_that_was_already_reading) {
    MockIn in;
    in.reading = true;
    {
        Reader r(&vtable, &in);
    }
    EXPECT_FALSE(in.hasCommit);
}

TEST(ReaderTest, does_commit_if_constructed_on_input_that_was_not_reading) {
    MockIn in;
    in.reading = false;
    {
        Reader r(&vtable, &in);
    }
    EXPECT_TRUE(in.hasCommit);
}

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
        EXPECT_FALSE(in.hasCommit);
    }
    EXPECT_TRUE(in.hasCommit);
}
