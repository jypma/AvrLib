#include <gtest/gtest.h>
#include "ChunkedFifo.hpp"

namespace ChunkedFifoTest {

using namespace Streams;

TEST(ChunkedFifoTest, empty_chunked_fifo_ignores_reads) {
    Fifo<16> data;
    ChunkedFifo f(data);

    uint8_t a = 123;
    EXPECT_FALSE(f.read(&a));
    EXPECT_EQ(123, a);
}

TEST(ChunkedFifoTest, chunked_fifo_ignores_too_big_writes) {
    Fifo<2> data;
    ChunkedFifo f(data);

    EXPECT_FALSE(f.write(FB(1,2,3)));
    EXPECT_EQ(0, data.getSize());
}

TEST(ChunkedFifoTest, chunked_fifo_ignores_too_big_reads) {
    Fifo<4> data;
    ChunkedFifo f(data);

    uint8_t in = 42;
    EXPECT_TRUE(f.write(in));
    EXPECT_EQ(2, data.getSize());

    uint8_t out1 = 123, out2 = 123;
    EXPECT_FALSE(f.read(&out1, &out2));

    uint8_t out = 123;
    EXPECT_TRUE(f.read(&out));
    EXPECT_EQ(42, out);
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, write_can_be_nested_in_explicit_writeStart) {
    Fifo<8> data;
    ChunkedFifo f(data);

    f.writeStart();
    f.write(uint8_t(42));
    f.write(uint8_t(84));
    EXPECT_TRUE(f.isWriting());
    EXPECT_EQ(0, f.getSize()); // in-progress write does not count to size
    f.writeEnd();
    EXPECT_FALSE(f.isWriting());

    EXPECT_EQ(3, data.getSize());

    uint8_t out1, out2;
    EXPECT_TRUE(f.read(&out1, &out2));
    EXPECT_EQ(42, out1);
    EXPECT_EQ(84, out2);
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, read_can_be_nested_in_explicit_readStart) {
    Fifo<8> data;
    ChunkedFifo f(data);

    f.writeStart();
    f.write(uint8_t(42));
    f.write(uint8_t(84));
    f.writeEnd();

    uint8_t out1 = 123;
    f.readStart();
    EXPECT_TRUE(f.read(&out1));
    uint8_t out2 = 123;
    EXPECT_TRUE(f.read(&out2));
    EXPECT_EQ(42, out1);
    EXPECT_EQ(84, out2);
    EXPECT_TRUE(f.isReading());
    f.readEnd();
    EXPECT_FALSE(f.isReading());
    EXPECT_EQ(0, f.getSize());
}

TEST(ChunkedFifoTest, ending_write_twice_has_no_ill_effects) {
    Fifo<2> data;
    ChunkedFifo f(data);

    EXPECT_FALSE(f.isWriting());

    f.writeStart();
    EXPECT_TRUE(f.isWriting());

    f.write(uint8_t(42));
    EXPECT_TRUE(f.isWriting());

    f.writeEnd();
    EXPECT_FALSE(f.isWriting());
    EXPECT_EQ(2, data.getSize());

    f.writeEnd();
    EXPECT_FALSE(f.isWriting());
    EXPECT_EQ(2, data.getSize());
}

TEST(ChunkedFifoTest, ending_read_twice_has_no_ill_effects) {
    Fifo<4> data;
    ChunkedFifo f(data);
    uint8_t in = 42;
    f.write(in);
    uint8_t in2 = 84;
    f.write(in2);

    EXPECT_EQ(4, data.getSize()); // 2 lengths, 2*1 data bytes
    EXPECT_FALSE(f.isReading());

    f.readStart();
    EXPECT_TRUE(f.isReading());

    uint8_t out = 123;
    f.read(&out);
    EXPECT_EQ(42, out);
    EXPECT_TRUE(f.isReading());

    f.readEnd();
    EXPECT_FALSE(f.isReading());
    EXPECT_EQ(2, data.getSize());

    f.readEnd();
    EXPECT_FALSE(f.isReading());
    EXPECT_EQ(2, data.getSize());
}

TEST(ChunkedFifoTest, a_write_with_full_fifo_is_ignored) {
    Fifo<2> data;
    ChunkedFifo f(data);
    uint8_t in = 42;
    f.write(in);

    EXPECT_EQ(2, data.getSize());
    f.writeStart();
    EXPECT_TRUE(f.isWriting());
    f.write(in);
    EXPECT_EQ(2, data.getSize());
    f.writeEnd();
    EXPECT_EQ(2, data.getSize());
    EXPECT_FALSE(f.isWriting());
}

TEST(ChunkedFifoTest, a_write_with_almost_full_fifo_is_ignored) {
    Fifo<3> data;
    ChunkedFifo f(data);
    uint8_t in = 42;
    f.write(in);

    EXPECT_EQ(2, data.getSize());
    f.write(in);
    EXPECT_EQ(2, data.getSize());
    EXPECT_FALSE(f.isWriting());
}

TEST(ChunkedFifoTest, aborting_a_write_does_not_conclude_it) {
    Fifo<2> data;
    ChunkedFifo f(data);
    f.writeStart();
    f.write(42);
    f.writeAbort();

    EXPECT_FALSE(f.isWriting());
    EXPECT_FALSE(f.isFull());
    EXPECT_FALSE(f.hasContent());
    EXPECT_EQ(0, data.getSize());
}

TEST(ChunkedFifoTest, aborting_a_read_does_not_conclude_it) {
    Fifo<2> data;
    ChunkedFifo f(data);
    uint8_t in = 42;
    f.write(in);

    f.readStart();
    uint8_t out = 123;
    f.read(&out);
    f.readAbort();

    ASSERT_TRUE(f.hasContent());
    ASSERT_EQ(2, data.getSize());
}

TEST(ChunkedFifoTest, fifo_can_handle_empty_chunks) {
    Fifo<2> data;
    ChunkedFifo f(data);
    EXPECT_FALSE(f.hasContent());
    f.writeStart();
    EXPECT_FALSE(f.hasContent());
    f.writeEnd();
    EXPECT_TRUE(f.hasContent());
    EXPECT_EQ(1, data.getSize());

    f.readStart();
    EXPECT_EQ(0, f.getReadAvailable());
    EXPECT_FALSE(f.hasContent());
    f.readEnd();
    EXPECT_FALSE(f.hasContent());
    EXPECT_EQ(0, data.getSize());
}

TEST(ChunkedFifoTest, early_ended_read_skips_over_remaining_data) {
    Fifo<16> data;
    ChunkedFifo fifo(data);
    fifo.writeStart();
    fifo.write(uint8_t(42));
    fifo.write(uint8_t(84));
    fifo.writeEnd();

    EXPECT_EQ(3, data.getSize());

    fifo.readStart();
    fifo.readEnd();

    EXPECT_FALSE(fifo.hasContent());
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, early_ended_reader_skips_over_remaining_data) {
    Fifo<16> data;
    ChunkedFifo fifo(data);
    fifo.writeStart();
    fifo.write(uint8_t(42));
    fifo.write(uint8_t(84));
    fifo.writeEnd();

    EXPECT_EQ(3, data.getSize());

    uint8_t length;
    fifo.read(&length);

    EXPECT_FALSE(fifo.hasContent());
    EXPECT_TRUE(data.isEmpty());
}

struct TestStruct {
    uint8_t a = 0, b = 0;
    TestStruct() {}
    TestStruct(uint8_t _a, uint8_t _b): a(_a), b(_b) {}
};

TEST(ChunkedFifoTest, throwaway_reader_removes_chunk) {
    Fifo<16> data;
    ChunkedFifo fifo(data);

    fifo.write(F("hello"));
    EXPECT_TRUE(fifo.hasContent());
    fifo.readStart();
    fifo.readEnd();
    EXPECT_FALSE(fifo.hasContent());
}

}
