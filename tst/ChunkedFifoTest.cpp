#include <gtest/gtest.h>
#include "ChunkedFifo.hpp"
#include "Streams/Streamable.hpp"

namespace ChunkedFifoTest {

using namespace Streams;

TEST(ChunkedFifoTest, empty_chunked_fifo_ignores_reads) {
    Fifo<16> data;
    ChunkedFifo f(&data);

    uint8_t a = 123;
    EXPECT_FALSE(f.in());
    EXPECT_FALSE(f.in() >> a);
    EXPECT_EQ(123, a);
}

TEST(ChunkedFifoTest, chunked_fifo_ignores_too_big_writes) {
    Fifo<2> data;
    ChunkedFifo f(&data);

    uint8_t a = 1, b = 2, c = 3;
    {
        auto writer = f.out();
        EXPECT_EQ(1, f.getSpace());
        writer << a;
        EXPECT_EQ(0, f.getSpace());
        writer << b;
        writer << c;
        EXPECT_FALSE(writer);
    }
    EXPECT_EQ(0, data.getSize());
}

TEST(ChunkedFifoTest, chunked_fifo_ignores_too_big_reads) {
    Fifo<4> data;
    ChunkedFifo f(&data);

    uint8_t in = 42;
    EXPECT_TRUE(f.out() << in);
    EXPECT_EQ(2, data.getSize());

    uint8_t out1 = 123, out2 = 123;
    EXPECT_FALSE(f.in() >> out1 >> out2);
    EXPECT_EQ(42, out1);
    EXPECT_EQ(123, out2);

    uint8_t out = 123;
    EXPECT_TRUE(f.in() >> out);
    EXPECT_EQ(42, out);
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, write_can_be_nested) {
    Fifo<3> data;
    ChunkedFifo f(&data);

    f.writeStart();
    f.write(42);
    f.writeStart();
    f.write(84);
    f.writeEnd();
    EXPECT_TRUE(f.isWriting());
    f.writeEnd();
    EXPECT_FALSE(f.isWriting());

    EXPECT_EQ(3, data.getSize());

    uint8_t out1, out2;
    EXPECT_TRUE(f.in() >> out1 >> out2);
    EXPECT_EQ(42, out1);
    EXPECT_EQ(84, out2);
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, read_can_be_nested) {
    Fifo<3> data;
    ChunkedFifo f(&data);

    f.writeStart();
    f.write(42);
    f.write(84);
    f.writeEnd();

    uint8_t out1 = 123;
    f.readStart();
    f.read(out1);
    uint8_t out2 = 123;
    f.readStart();
    f.read(out2);
    EXPECT_EQ(42, out1);
    EXPECT_EQ(84, out2);
    f.readEnd();
    EXPECT_TRUE(f.isReading());
    f.readEnd();
    EXPECT_FALSE(f.isReading());
}

TEST(ChunkedFifoTest, ending_write_twice_has_no_ill_effects) {
    Fifo<2> data;
    ChunkedFifo f(&data);

    EXPECT_FALSE(f.isWriting());

    f.writeStart();
    EXPECT_TRUE(f.isWriting());

    f.write(42);
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
    ChunkedFifo f(&data);
    uint8_t in = 42;
    f.out() << in;
    uint8_t in2 = 84;
    f.out() << in2;

    EXPECT_EQ(4, data.getSize()); // 2 lengths, 2*1 data bytes
    EXPECT_FALSE(f.isReading());

    f.readStart();
    EXPECT_TRUE(f.isReading());

    uint8_t out = 123;
    f.read(out);
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
    ChunkedFifo f(&data);
    uint8_t in = 42;
    f.out() << in;

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
    ChunkedFifo f(&data);
    uint8_t in = 42;
    f.out() << in;

    EXPECT_EQ(2, data.getSize());
    f.writeStart();
    EXPECT_TRUE(f.isWriting());
    f.write(in);
    EXPECT_EQ(2, data.getSize());
    f.writeEnd();
    EXPECT_EQ(2, data.getSize());
    EXPECT_FALSE(f.isWriting());
}

TEST(ChunkedFifoTest, aborting_a_write_does_not_conclude_it) {
    Fifo<2> data;
    ChunkedFifo f(&data);
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
    ChunkedFifo f(&data);
    uint8_t in = 42;
    f.out() << in;

    f.readStart();
    uint8_t out = 123;
    f.read(out);
    f.readAbort();

    ASSERT_TRUE(f.hasContent());
    ASSERT_EQ(2, data.getSize());
}

TEST(ChunkedFifoTest, fifo_can_handle_empty_chunks) {
    Fifo<2> data;
    ChunkedFifo f(&data);
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

TEST(ChunkedFifoTest, anonymous_writer_applies_after_leaving_scope) {
    Fifo<16> data;
    ChunkedFifo fifo(&data);

    EXPECT_TRUE(fifo.out() << uint16_t(4200) << uint8_t(84));
    EXPECT_EQ(4, data.getSize());
}

TEST(ChunkedFifoTest, early_ended_read_skips_over_remaining_data) {
    Fifo<16> data;
    ChunkedFifo fifo(&data);
    fifo.writeStart();
    fifo.write(42);
    fifo.write(84);
    fifo.writeEnd();

    EXPECT_EQ(3, data.getSize());

    fifo.readStart();
    fifo.readEnd();

    EXPECT_FALSE(fifo.hasContent());
    EXPECT_TRUE(data.isEmpty());
}

TEST(ChunkedFifoTest, early_ended_reader_skips_over_remaining_data) {
    Fifo<16> data;
    ChunkedFifo fifo(&data);
    fifo.writeStart();
    fifo.write(42);
    fifo.write(84);
    fifo.writeEnd();

    EXPECT_EQ(3, data.getSize());

    {
        uint8_t length;
        fifo.in() >> length;
    }

    EXPECT_FALSE(fifo.hasContent());
    EXPECT_TRUE(data.isEmpty());
}

struct TestStruct: public Streamable<TestStruct> {
    uint8_t a = 0, b = 0;
    TestStruct() {}
    TestStruct(uint8_t _a, uint8_t _b): a(_a), b(_b) {}

    typedef Format<
        Scalar<uint8_t, &TestStruct::b>,
        Scalar<uint8_t, &TestStruct::a>
    > Proto;
};

TEST(ChunkedFifoTest, chunked_fifo_is_stream_api_compatible) {

    Fifo<16> data;
    ChunkedFifo fifo(&data);
    {
        Writer<ChunkedFifo> out(fifo);
        out << uint8_t(42);
        out << TestStruct(1, 2);
    }

    {
        uint8_t a;
        TestStruct foo;
        Reader<ChunkedFifo> in(fifo);
        in >> a;
        in >> foo;
        EXPECT_TRUE(in);
    }
}

}
