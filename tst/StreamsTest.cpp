#include <gtest/gtest.h>
#include "Fifo.hpp"
#include "ChunkedFifo.hpp"
#include "Streams/Reader.hpp"
#include "Streams/Writer.hpp"
#include "Streams/Streamable.hpp"
#include "Espressif/EthernetMACAddress.hpp"

namespace StreamsTest {

using namespace Streams;
using namespace Espressif;

struct TestStruct: public Streamable<TestStruct> {
    uint8_t a = 0, b = 0;
    TestStruct() {}
    TestStruct(uint8_t _a, uint8_t _b): a(_a), b(_b) {}

    typedef Format<
        Binary<uint8_t, &TestStruct::b>,
        Binary<uint8_t, &TestStruct::a>
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

struct MockFifo {
    int getSpaceCount = 0;

    inline void writeStart() {}
    inline void writeEnd() {}
    inline void writeAbort() {}

    inline int getSpace() {
        getSpaceCount++;
        return 2;
    }

    inline void uncheckedWrite(uint8_t value) {}

    int getReadAvailableCount = 0;

    inline void readStart() {}
    inline void readEnd() {}
    inline void readAbort() {}

    inline int getReadAvailable() {
        getReadAvailableCount++;
        return 2;
    }

    inline void uncheckedRead(uint8_t &target) {}
};

TEST(StreamsTest, writing_constant_size_format_only_checks_available_size_once) {
    MockFifo fifo;
    {
        Writer<MockFifo> out(fifo);
        out << TestStruct(42, 84);
    }

    EXPECT_EQ(1, fifo.getSpaceCount);
}

TEST(StreamsTest, format_consisting_of_only_literals_has_fixed_size) {
    typedef Parts::Format<TestStruct,
            Parts::Binary<TestStruct,uint8_t, &TestStruct::b>,
            Parts::Binary<TestStruct,uint8_t, &TestStruct::a>
        > P;

    static_assert(P::fixedSize == 2, "two bytes hav esize 2");
}

TEST(StreamsTest, reading_constant_size_format_only_checks_available_size_once) {
    MockFifo fifo;
    {
        Reader<MockFifo> in(fifo);
        TestStruct s;
        in >> s;
    }

    EXPECT_EQ(1, fifo.getReadAvailableCount);
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
        Binary<uint8_t, &TestStruct3::a>,
        Conditional<&TestStruct3::hasBigA,
            Binary<uint8_t, &TestStruct3::b>
        >
    > Proto;
};

TEST(StreamsTest, conditional_field_is_included_on_true) {
    auto fifo = Fifo<2>();
    fifo.out() << TestStruct3(101, 42);
    EXPECT_EQ(2, fifo.getSize());

    TestStruct3 returned;
    EXPECT_TRUE(fifo.in() >> returned);
    EXPECT_EQ(101, returned.a);
    EXPECT_EQ(42, returned.b);
}

TEST(StreamsTest, struct_with_absent_conditional_field_can_fit_in_fifo_even_if_couditional_would_not) {
    auto fifo = Fifo<1>();
    EXPECT_EQ(1, fifo.getSpace());
    EXPECT_TRUE(fifo.out() << TestStruct3(0, 42));
    EXPECT_EQ(0, fifo.getSpace());
    EXPECT_EQ(1, fifo.getSize());
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

TEST(StreamsTest, conditional_field_aborts_read_if_fifo_would_run_out) {
    struct T: public Streamable<T> {
        uint8_t ch1 = 0, ch2 = 0;

        bool isCh1A() const { return ch1 == 'a'; }

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "a";

            EXPECT_EQ(ReaderState::Incomplete, (fifo.readAs<Format<
                Binary<uint8_t, &T::ch1>,
                Conditional<&T::isCh1A,
                    Binary<uint8_t, &T::ch2>
                >
            >>(*this)));

            EXPECT_EQ('a', ch1);
            EXPECT_EQ(0, ch2);
        }
    } t;
}

TEST(StreamsTest, absense_of_token_fails_reader) {
    struct T: public Streamable<T> {
        uint8_t ch = 0;

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "abcdef";

            EXPECT_EQ(ReaderState::Invalid, (fifo.readAs<Format<
                Token<STR("aaa")>,
                Binary<uint8_t, &T::ch>
            >>(*this)));

            EXPECT_EQ(0, ch);
        }
    } t;
}

TEST(StreamsTest, char_after_token_is_read) {
    struct T: public Streamable<T> {
        uint8_t ch = 0;

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "bcdef";

            EXPECT_EQ(ReaderState::Valid, (fifo.readAs<Format<
                Token<STR("bcd")>,
                Binary<uint8_t, &T::ch>
            >>(*this)));

            EXPECT_EQ('e', ch);
        }
    } t;

}

TEST(StreamsTest, can_parse_mac_address) {
    auto fifo = Fifo<24>();
    fifo.out() << "06:0f:A0:de:21:4f";
    EthernetMACAddress address;
    EXPECT_TRUE(fifo.in() >> address);
    EXPECT_EQ(0x06, address.byte1());
    EXPECT_EQ(0x0F, address.byte2());
    EXPECT_EQ(0xA0, address.byte3());
    EXPECT_EQ(0xDE, address.byte4());
    EXPECT_EQ(0x21, address.byte5());
    EXPECT_EQ(0x4F, address.byte6());
}

TEST(StreamsTest, can_write_mac_address) {
    EthernetMACAddress address = { 0xF0, 0x34, 0x02, 0x4A, 0xDE, 0x00 };
    auto fifo = Fifo<24>();
    fifo.out() << address;
    EXPECT_TRUE((fifo.in().expect<Seq<Token<STR("F0:34:02:4A:DE:00")>>>()));
}

TEST(StreamTest, partially_present_token_marks_reader_as_partial) {
    struct T: public Streamable<T> {
        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "ab";

            EXPECT_EQ(ReaderState::Partial, (fifo.readAs<Format<
                Token<STR("abc")>
            >>(*this)));
        }
    } t;
}

}
