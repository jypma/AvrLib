#include <gtest/gtest.h>
#include "Fifo.hpp"
#include "Streams/Reader.hpp"
#include "Streams/Writer.hpp"
#include "Streams/Streamable.hpp"

namespace StreamsTest {

using namespace Streams;

struct TestStruct: public Streamable<TestStruct> {
    uint8_t a = 0, b = 0;
    TestStruct() {}
    TestStruct(uint8_t _a, uint8_t _b): a(_a), b(_b) {}

    typedef Format<
        Scalar<uint8_t, &TestStruct::b>,
        Scalar<uint8_t, &TestStruct::a>
    > Proto;
};

struct TestStruct2: public Streamable<TestStruct2> {
    uint8_t a[5];

    TestStruct2() {}

    TestStruct2(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
        a[0] = a0;
        a[1] = a1;
        a[2] = a2;
        a[3] = a3;
        a[4] = a4;
    }

    typedef Format<
        Array<uint8_t, 5, &TestStruct2::a>
    > Proto;
};

TEST(StreamsTest, byte_fields_can_be_written) {
   auto fifo = Fifo<17>();
   fifo.out() << TestStruct(42, 84);

   EXPECT_EQ(2, fifo.getSize());
   uint8_t b1, b2;
   fifo.read(b1);
   fifo.read(b2);

   EXPECT_EQ(84, b1);
   EXPECT_EQ(42, b2);
}

TEST(StreamsTest, multiple_byte_fields_can_be_read) {
    auto fifo = Fifo<6>();
    fifo.write(uint8_t(84));
    fifo.write(uint8_t(42));
    fifo.write(uint8_t(42));
    fifo.write(uint8_t(84));
    fifo.write(uint8_t(42));
    fifo.write(uint8_t(84));

    TestStruct s, s2, s3;
    EXPECT_TRUE(fifo.in() >> s >> s2 >> s3);
    EXPECT_FALSE(fifo.isReading());
    EXPECT_EQ(0, fifo.getSize()); // read should have committed
    EXPECT_EQ(42, s.a);
    EXPECT_EQ(84, s.b);
    EXPECT_EQ(84, s2.a);
    EXPECT_EQ(42, s2.b);
    EXPECT_EQ(84, s3.a);
    EXPECT_EQ(42, s3.b);
}

TEST(StreamsTest, byte_fields_can_be_read) {
    auto fifo = Fifo<6>();
    fifo.write(uint8_t(84));
    fifo.write(uint8_t(42));

    TestStruct s;
    EXPECT_TRUE(fifo.in() >> s);
    EXPECT_FALSE(fifo.isReading());
    EXPECT_EQ(0, fifo.getSize()); // read should have committed
    EXPECT_EQ(42, s.a);
    EXPECT_EQ(84, s.b);
}

TEST(StreamsTest, array_fields_can_be_written) {
   auto fifo = Fifo<17>();
   fifo.out() << TestStruct2(42, 43, 44, 45, 46);

   EXPECT_EQ(5, fifo.getSize());
   uint8_t b1, b2, b3, b4, b5;
   fifo.read(b1);
   fifo.read(b2);
   fifo.read(b3);
   fifo.read(b4);
   fifo.read(b5);

   EXPECT_EQ(42, b1);
   EXPECT_EQ(43, b2);
   EXPECT_EQ(44, b3);
   EXPECT_EQ(45, b4);
   EXPECT_EQ(46, b5);
}

TEST(StreamsTest, array_fields_can_be_read) {
   auto fifo = Fifo<17>();
   fifo.write(42);
   fifo.write(43);
   fifo.write(44);
   fifo.write(45);
   fifo.write(46);

   TestStruct2 t;
   fifo.in() >> t;

   EXPECT_EQ(42, t.a[0]);
   EXPECT_EQ(43, t.a[1]);
   EXPECT_EQ(44, t.a[2]);
   EXPECT_EQ(45, t.a[3]);
   EXPECT_EQ(46, t.a[4]);
}

struct TestStruct3: public Streamable<TestStruct3> {
    uint8_t a = 0, b = 0;
    TestStruct3() {}
    TestStruct3(uint8_t _a, uint8_t _b): a(_a), b(_b) {}

    bool hasBigA() const { return a > 100; }

    typedef Format<
        Scalar<uint8_t, &TestStruct3::a>,
        Conditional<&TestStruct3::hasBigA,
            Scalar<uint8_t, &TestStruct3::b>
        >
    > Proto;
};

TEST(StreamsTest, conditional_field_is_included_on_true) {
    constexpr int min = TestStruct3::Proto::minimumSize;
    EXPECT_EQ(1, min);
    constexpr int max = TestStruct3::Proto::maximumSize;
    EXPECT_EQ(2, max);

    auto fifo = Fifo<2>();
    fifo.out() << TestStruct3(101, 42);
    EXPECT_EQ(2, fifo.getSize());

    TestStruct3 returned;
    EXPECT_TRUE(fifo.in() >> returned);
    EXPECT_EQ(101, returned.a);
    EXPECT_EQ(42, returned.b);
}

TEST(StreamTest, maximum_size_determines_whether_instance_is_written_to_fifo) {
    auto fifo = Fifo<1>();
    EXPECT_EQ(1, fifo.getSpace());
    EXPECT_FALSE(fifo.out() << TestStruct3(0, 42));
    EXPECT_EQ(0, fifo.getSize());
}

TEST(StreamsTest, conditional_field_is_excluded_on_false) {
    auto fifo = Fifo<2>();
    fifo.out() << TestStruct3(99, 42);
    EXPECT_EQ(1, fifo.getSize());

    TestStruct3 returned;
    EXPECT_TRUE(fifo.in() >> returned);
    EXPECT_EQ(99, returned.a);
    EXPECT_EQ(0, returned.b);
}

}
