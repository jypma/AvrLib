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

/*
TEST(ReaderTest, scanning_for_token_finds_token) {
    Fifo<16> fifo;
    fifo.out() << "abcde";
    uint8_t i;
    EXPECT_TRUE(fifo.in() >> "bc" >> i);
    EXPECT_EQ('d', i);
    EXPECT_EQ(1, fifo.getSize());
}

TEST(ReaderTest, scanning_for_token_fails_if_not_present) {
    Fifo<16> fifo;
    fifo.out() << "abcde";
    uint8_t i;
    EXPECT_FALSE(fifo.in() >> "bz" >> i);
    EXPECT_EQ(5, fifo.getSize());
}

TEST(ReaderTest, scanning_for_nullptr_is_ignored_but_does_not_fail_parsing) {
    Fifo<16> fifo;
    fifo.out() << "abcde";
    EXPECT_TRUE(fifo.in() >> (const char *)(nullptr));
    EXPECT_EQ(5, fifo.getSize());

}

TEST(Readertest, scanning_for_token_longer_than_input_fails) {
    Fifo<16> fifo;
    fifo.out() << "abcde";
    uint8_t i;
    EXPECT_FALSE(fifo.in() >> "abcdef" >> i);
    EXPECT_EQ(5, fifo.getSize());
}
*/
}
