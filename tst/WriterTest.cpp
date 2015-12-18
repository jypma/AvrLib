#include <gtest/gtest.h>
#include "Streams/Writer.hpp"
#include "EEPROMTest.hpp"
#include "ChunkedFifo.hpp"

namespace WriterTest {

using namespace Streams;

struct MockFifo {
    uint8_t buffer[200];
    uint8_t length = 0;
    uint8_t space = 200;
    uint8_t pretendFull = 0;
    uint8_t isFullInvocations = 0;

    bool started = false;
    bool ended = false;
    bool aborted = false;

    inline bool isFull() {
        if (pretendFull > 0) {
            pretendFull--;
            isFullInvocations++;
            return true;
        } else {
            return false;
        }
    }
    void writeStart() {
        started = true;
    }
    void writeEnd() {
        ended = true;
    }
    void writeAbort() {
        aborted = true;
    }
    uint8_t getSpace() const {
        return space;
    }
    void uncheckedWrite(uint8_t ch) {
        if (pretendFull == 0) {
            buffer[length] = ch;
            length++;
            space--;
        }
    }
    void write(uint8_t ch) {
        uncheckedWrite(ch);
    }

};

TEST(WriterTest, write_const_string_should_not_include_terminating_zero) {
    MockFifo f;
    auto out = Writer<MockFifo>(f);
    out << "hello";
    EXPECT_EQ(5, f.length);
}

TEST(WriterTest, blocking_semantics_writer_should_immediately_commit_all_writes) {
    SREG |= SREG_I;

    MockFifo f;
    auto out = Writer<MockFifo, BlockingWriteSemantics<MockFifo>>(f);
    EXPECT_FALSE(f.started);
    EXPECT_FALSE(f.ended);
    EXPECT_FALSE(f.aborted);

    out << uint8_t(42);
    EXPECT_FALSE(f.started);
    EXPECT_FALSE(f.ended);
    EXPECT_FALSE(f.aborted);
}

TEST(WriterTest, noblocking_semantics_are_default_and_dont_commit_writes_until_writer_goes_out_of_scope) {
    MockFifo f;
    {
        auto out = Writer<MockFifo>(f);
        EXPECT_TRUE(f.started);
        EXPECT_FALSE(f.ended);
        EXPECT_FALSE(f.aborted);

        out << uint8_t(42);
        EXPECT_FALSE(f.ended);
        EXPECT_FALSE(f.aborted);
    }

    EXPECT_TRUE(f.ended);
    EXPECT_FALSE(f.aborted);
}

enum class TestEnum: uint8_t {ONE, TWO};

TEST(WriterTest, raw_bytes_and_enums_are_output_correctly) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        EXPECT_TRUE(out.started);
        w << uint8_t(42);
        w << uint8_t(84);
        w << TestEnum::ONE;
        w << TestEnum::TWO;
        EXPECT_FALSE(out.ended);
    }
    EXPECT_EQ(4, out.length);
    EXPECT_EQ(42, out.buffer[0]);
    EXPECT_EQ(84, out.buffer[1]);
    EXPECT_EQ(0, out.buffer[2]);
    EXPECT_EQ(1, out.buffer[3]);
    EXPECT_TRUE(out.ended);
}

TEST(WriterTest, uint16_is_output_lsb_first) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << uint16_t(0xFFEE);
    }
    EXPECT_EQ(0xEE, out.buffer[0]);
    EXPECT_EQ(0xFF, out.buffer[1]);
    EXPECT_EQ(2, out.length);
}

TEST(WriterTest, uint32_is_output_lsb_first) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << uint32_t(0xFFEEDDCC);
    }
    EXPECT_EQ(0xCC, out.buffer[0]);
    EXPECT_EQ(0xDD, out.buffer[1]);
    EXPECT_EQ(0xEE, out.buffer[2]);
    EXPECT_EQ(0xFF, out.buffer[3]);
    EXPECT_EQ(4, out.length);
}

TEST(WriterTest, string_is_outputted) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << "42";
    }
    EXPECT_EQ('4', out.buffer[0]);
    EXPECT_EQ('2', out.buffer[1]);
    EXPECT_EQ(2, out.length);
}

TEST(WriterTest, decimal_uint8_is_handled) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
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

TEST(WriterTest, pointers_can_be_written) {
    uint8_t i = 42;
    uint8_t *ptr = &i;

    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << ptr;
    }
    EXPECT_TRUE(out.length > 1);
}

TEST(WriterTest, decimal_int8_is_handled) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
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
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
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
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
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

TEST(WriterTest, decimal_uint32_t_is_handled) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << dec(uint32_t(0));
        w << dec(uint32_t(4002));
        w << dec(uint32_t(4294967295));
    }
    EXPECT_EQ('0', out.buffer[0]);
    EXPECT_EQ('4', out.buffer[1]);
    EXPECT_EQ('0', out.buffer[2]);
    EXPECT_EQ('0', out.buffer[3]);
    EXPECT_EQ('2', out.buffer[4]);
    EXPECT_EQ('4', out.buffer[5]);
    EXPECT_EQ('2', out.buffer[6]);
    EXPECT_EQ('9', out.buffer[7]);
    EXPECT_EQ('4', out.buffer[8]);
    EXPECT_EQ('9', out.buffer[9]);
    EXPECT_EQ('6', out.buffer[10]);
    EXPECT_EQ('7', out.buffer[11]);
    EXPECT_EQ('2', out.buffer[12]);
    EXPECT_EQ('9', out.buffer[13]);
    EXPECT_EQ('5', out.buffer[14]);
    EXPECT_EQ(15, out.length);
}

TEST(WriterTest, decimal_from_eeprom_is_possible) {
    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        w << dec(&EEPROM::data);
        w << dec(&EEPROM::remotePort);
    }
}

TEST(WriterTest, can_write_chunk_from_chunked_fifo_as_reader) {
    Fifo<200> inputData;
    ChunkedFifo input(&inputData);

    input.out() << "hello";

    MockFifo out;
    {
        auto w = Writer<MockFifo>(out);
        auto in = input.in();
        if (in.getReadAvailable() > 0) {
            w << in;
        }
    }

    EXPECT_EQ(5, out.length);
    EXPECT_EQ('h', out.buffer[0]);
    EXPECT_EQ('o', out.buffer[4]);
}

TEST(WriterTest, blocking_write_semantics_block_until_fifo_no_longer_full) {
    MockFifo out;
    SREG = (1 << SREG_I);
    out.pretendFull = 10;
    {
        auto w = Writer<MockFifo,BlockingWriteSemantics<MockFifo>>(out);
        w << dec(uint8_t(102));
    }

    EXPECT_EQ(10, out.isFullInvocations);
    EXPECT_EQ(3, out.length);
    EXPECT_EQ('1', out.buffer[0]);
    EXPECT_EQ('0', out.buffer[1]);
    EXPECT_EQ('2', out.buffer[2]);
}

TEST(WriterTest, blocking_write_semantics_dont_block_when_interrupts_are_off) {
    MockFifo out;
    SREG = 0;
    out.pretendFull = 10;
    {
        auto w = Writer<MockFifo,BlockingWriteSemantics<MockFifo>>(out);
        w << dec(uint8_t(102));
    }

    EXPECT_EQ(0, out.isFullInvocations);
    EXPECT_EQ(0, out.length);
}

}
