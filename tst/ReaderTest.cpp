#include <gtest/gtest.h>
#include "Streams/Reader.hpp"
#include "Fifo.hpp"

namespace ReaderTest {

using namespace Streams;

struct MockIn {
    uint8_t buffer[200];
    uint8_t remaining = 0;
    uint8_t pos = 0;

    bool started = false;
    bool ended = false;
    bool aborted = false;

    void readStart() {
        started = true;
    }
    void readEnd() {
        ended = true;
    }
    void readAbort() {
        aborted = true;
    }
    void uncheckedRead(uint8_t &ch) {
        ch = buffer[pos];
        pos++;
        remaining--;
    }
    uint8_t getReadAvailable() const {
        return remaining;
    }
};

enum class ReaderTestEnum: uint8_t {ONE, TWO};

TEST(ReaderTest, raw_bytes_are_read_correctly) {
    MockIn in;
    in.buffer[0] = 42;
    in.buffer[1] = 1;
    in.remaining = 2;
    {
        auto r = Reader<MockIn>(in);
        EXPECT_TRUE(in.started);
        EXPECT_EQ(2, r.getReadAvailable());
        uint8_t inbyte;
        r >> inbyte;
        EXPECT_EQ(42, inbyte);
        //ReaderTestEnum inenum;
        //r >> inenum;
        //EXPECT_EQ(ReaderTestEnum::TWO, inenum);
        EXPECT_FALSE(in.ended);
    }
    EXPECT_TRUE(in.ended);
}

TEST(ReaderTest, multibyte_ints_are_read_correctly) {
    MockIn in;
    in.buffer[0] = 0xDE;
    in.buffer[1] = 0xAD;
    in.remaining = 2;
    {
        auto r = Reader<MockIn>(in);
        uint16_t inbyte;
        r >> inbyte;
        EXPECT_EQ(0xADDE, inbyte);
    }
}
}
