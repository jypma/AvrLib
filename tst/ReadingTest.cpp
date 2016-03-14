#include <gtest/gtest.h>
#include "Fifo.hpp"
#include "ChunkedFifo.hpp"

namespace ReadingTest {

using namespace Streams;

TEST(ReadingTest, fifo_can_read_ints) {
    Fifo<16> fifo;
    for (uint8_t i = 0; i < 14; i++) {
        fifo.write(i);
    }

    uint8_t uint8;
    int8_t int8;
    uint16_t uint16;
    int16_t int16;
    uint32_t uint32;
    int32_t int32;
    EXPECT_TRUE(fifo.read(&uint8, &int8, &uint16, &int16, &uint32, &int32));
    EXPECT_EQ(0x00, uint8);
    EXPECT_EQ(0x01, int8);
    EXPECT_EQ(0x0302, uint16);   // little endian -> LSByte is read first
    EXPECT_EQ(0x0504, int16);
    EXPECT_EQ(0x09080706, uint32);
    EXPECT_EQ(0x0D0C0B0A, int32);
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_and_verify_progmem_strings) {
    Fifo<16> fifo;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(F("foo")));

    fifo.write('f');
    EXPECT_EQ(ReadResult::Partial, fifo.read(F("foo")));
    EXPECT_EQ(ReadResult::Invalid, fifo.read(F("c")));
    EXPECT_EQ(ReadResult::Invalid, fifo.read(F("cow")));

    fifo.write('o');
    fifo.write('o');
    EXPECT_EQ(ReadResult::Valid, fifo.read(F("foo")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(ReadingTest, fifo_can_read_and_verify_int_token_literals) {
    Fifo<16> fifo;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(FB(1,2,3)));
    EXPECT_FALSE(fifo.isReading());
    fifo.write(uint8_t(1));
    EXPECT_EQ(ReadResult::Partial, fifo.read(FB(1,2,3)));
    EXPECT_FALSE(fifo.isReading());
    fifo.write(uint8_t(2));
    fifo.write(uint8_t(3));
    EXPECT_EQ(ReadResult::Valid, fifo.read(FB(1,2,3)));
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_decimal_ints) {
    Fifo<16> fifo;
    uint8_t int8;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(dec(&int8)));

    fifo.write('0');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(0, int8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('1');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(1, int8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('0');
    fifo.write('f');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(0, int8);
    EXPECT_EQ(1, fifo.getReadAvailable());
    fifo.clear();

    fifo.write('1');
    fifo.write('f');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(1, int8);
    EXPECT_EQ(1, fifo.getReadAvailable());
    fifo.clear();

    fifo.write('1');
    fifo.write('0');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(10, int8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('1');
    fifo.write('1');
    fifo.write('f');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(11, int8);
    EXPECT_EQ(1, fifo.getReadAvailable());
    fifo.clear();

    fifo.write('2');
    fifo.write('5');
    fifo.write('5');
    EXPECT_EQ(ReadResult::Valid, fifo.read(dec(&int8)));
    EXPECT_EQ(255, int8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('2');
    fifo.write('5');
    fifo.write('6');
    EXPECT_EQ(ReadResult::Invalid, fifo.read(dec(&int8)));
    EXPECT_EQ(3, fifo.getReadAvailable());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_padding) {
    Fifo<16> fifo;
    EXPECT_EQ(ReadResult::Valid, fifo.read(Padding(0)));
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(Padding(1)));
    EXPECT_FALSE(fifo.isReading());

    fifo.write('1');
    EXPECT_EQ(ReadResult::Valid, fifo.read(Padding(0)));
    EXPECT_EQ(1, fifo.getReadAvailable());
    EXPECT_EQ(ReadResult::Valid, fifo.read(Padding(1)));
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('4');
    fifo.write('2');
    EXPECT_EQ(ReadResult::Valid, fifo.read(Padding(2)));
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

struct MyStruct {
    typedef Protocol<MyStruct> P;

    uint8_t uint8;

    typedef P::Seq<
      P::Binary<uint8_t, &MyStruct::uint8>
    > DefaultProtocol;
};

struct MyStruct2 {
    MyStruct nested;

    typedef Protocol<MyStruct2> P;
    typedef P::Seq<
        P::Object<MyStruct, &MyStruct2::nested>,
        P::Object<MyStruct, &MyStruct2::nested, MyStruct::DefaultProtocol>
    > DefaultProtocol;
};

TEST(ReadingTest, fifo_can_read_struct_with_DefaultProtocol) {
    Fifo<16> fifo;
    fifo.write('4');
    MyStruct s;
    EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
    EXPECT_EQ('4', s.uint8);
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_struct_with_specific_protocol) {
    Fifo<16> fifo;
    fifo.write('4');
    MyStruct s;
    typedef Protocol<MyStruct> P;
    EXPECT_EQ(ReadResult::Valid, fifo.read(as<P::Binary<uint8_t, &MyStruct::uint8>>(&s)));
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('5');
    typedef P::Seq<P::Padding<1>, P::Binary<uint8_t, &MyStruct::uint8>> FMT;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(as<FMT>(&s)));
    EXPECT_FALSE(fifo.isReading());

    fifo.write('6');
    EXPECT_EQ(ReadResult::Valid, fifo.read(as<FMT>(&s)));
    EXPECT_EQ('6', s.uint8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write('a');
    fifo.write('1');
    EXPECT_EQ(ReadResult::Invalid, fifo.read(as<STR("b"), P::Binary<uint8_t, &MyStruct::uint8>>(&s)));
    EXPECT_EQ(2, fifo.getReadAvailable());
    EXPECT_EQ(ReadResult::Valid,   fifo.read(as<STR("a"), P::Binary<uint8_t, &MyStruct::uint8>>(&s)));
    EXPECT_EQ('1', s.uint8);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_struct_with_nested_protocol) {
    Fifo<16> fifo;
    fifo.write('1');
    fifo.write('2');
    MyStruct2 s;
    EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
    EXPECT_EQ('2', s.nested.uint8);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

struct MyStruct3 {
    uint8_t a, b;

    bool hasB() const { return a > 0; }

    typedef Protocol<MyStruct3> P;
    typedef P::Seq<
      P::Binary<uint8_t, &MyStruct3::a>,
      P::Conditional<&MyStruct3::hasB,
        P::Binary<uint8_t, &MyStruct3::b>
      >
    > DefaultProtocol;
};

TEST(ReadingTest, fifo_can_read_struct_with_conditional_protocol) {
    Fifo<16> fifo;
    MyStruct3 s;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(&s));
    EXPECT_FALSE(fifo.isReading());
    fifo.write('1');
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(&s));
    EXPECT_FALSE(fifo.isReading());
    fifo.write('2');
    EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
    EXPECT_FALSE(fifo.isReading());
    EXPECT_EQ('1', s.a);
    EXPECT_EQ('2', s.b);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write(uint8_t(0));
    EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
    EXPECT_EQ(0, s.a);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, fifo_can_read_conditionally) {
    Fifo<16> fifo;
    fifo.write('1');
    fifo.write('2');
    uint8_t a, b;

    EXPECT_EQ(ReadResult::Valid, fifo.read(&a, Nested([&] (auto read) { return (a == '1') ? read(&b) : ReadResult::Valid; })));
    EXPECT_EQ('1', a);
    EXPECT_EQ('2', b);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, absent_conditional_does_not_fail_read) {
    Fifo<16> fifo;
    uint8_t a, b;
    fifo.write('3');
    EXPECT_EQ(ReadResult::Valid, fifo.read(&a, Nested([&] (auto read) { return (a == '1') ? read(&b) : ReadResult::Valid; })));
    EXPECT_EQ('3', a);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, failure_inside_conditional_rolls_back_entire_read) {
    Fifo<16> fifo;
    uint8_t a, b;
    fifo.write('1');
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(&a, Nested([&] (auto read) { return (a == '1') ? read(&b) : ReadResult::Valid; })));
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, can_read_decimal_length_and_bytes_into_chunked_fifo) {
    Fifo<16> src;
    Fifo<32> dest_storage;
    ChunkedFifo dest(dest_storage);

    auto doIt = Nested([&] (auto read) -> ReadResult {
        uint8_t length;
        const auto result = read(Decimal(&length), F(":"));
        if (result) {
            if (src.getReadAvailable() >= length) {
                //once ChunkedFifo gets the treatment
                //dest.write(From(src, length));
                dest.writeStart();
                // TODO optimize into memcpy
                for (uint8_t i = 0; i < length; i++) {
                    uint8_t b;
                    src.uncheckedRead(b);
                    dest.write(b);
                }
                dest.writeEnd();
                return ReadResult::Valid;
            } else {
                return ReadResult::Incomplete;
            }
        } else {
            return result;
        }
    });

    src.write('5');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(1, src.getSize());
    src.write(':');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(2, src.getSize());
    src.write('1');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(3, src.getSize());
    src.write('2');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(4, src.getSize());
    src.write('3');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(5, src.getSize());
    src.write('4');
    EXPECT_EQ(ReadResult::Incomplete, src.read(doIt));
    EXPECT_EQ(6, src.getSize());
    src.write('5');
    EXPECT_EQ(ReadResult::Valid, src.read(doIt));
    EXPECT_TRUE(src.isEmpty());
    EXPECT_FALSE(src.isReading());
    EXPECT_EQ(6, dest.getSize());
    uint8_t b[5];
    EXPECT_TRUE(dest.read(&b[0], &b[1], &b[2], &b[3], &b[4]));
    EXPECT_EQ('1', b[0]);
    EXPECT_EQ('2', b[1]);
    EXPECT_EQ('3', b[2]);
    EXPECT_EQ('4', b[3]);
    EXPECT_EQ('5', b[4]);
}

TEST(ReadingTest, read_rolls_back_on_incomplete_fifo) {
    Fifo<8> fifo;
    fifo.write(F("hel"));
    uint8_t a,b,c,d;
    EXPECT_FALSE(fifo.read(&a, &b, &c, &d));
    EXPECT_EQ(3, fifo.getSize());
    EXPECT_FALSE(fifo.isReading());
}

TEST(ReadingTest, reading_chunk_completes_once_bytes_are_available) {
    Fifo<20> src;
    Fifo<20> destData;
    ChunkedFifo dest(destData);

    src.write(F("5:abcd"));
    uint8_t length;
    dest.writeStart();
    EXPECT_EQ(ReadResult::Incomplete, src.read(Decimal(&length), F(":"), ChunkWithLength(&length, dest)));
    EXPECT_TRUE(dest.isWriting()); // should still be writing after a failed read
    src.write(F("e"));

    EXPECT_EQ(ReadResult::Valid,      src.read(Decimal(&length), F(":"), ChunkWithLength(&length, dest)));
    EXPECT_TRUE(dest.isWriting()); // should still be writing after a successful read as well
    dest.writeEnd();

    EXPECT_TRUE(src.isEmpty());
    EXPECT_EQ(6, dest.getSize());
}

TEST(ReadingTest, reading_chunk_is_dropped_if_no_space_in_target_fifo) {
    Fifo<20> src;
    Fifo<5> destData;
    ChunkedFifo dest(destData);

    src.write(F("5:abcde"));
    dest.writeStart();
    uint8_t length;
    EXPECT_EQ(ReadResult::Valid, src.read(Decimal(&length), F(":"), ChunkWithLength(&length, dest)));
    EXPECT_TRUE(src.isEmpty());
    EXPECT_TRUE(dest.isEmpty());
    EXPECT_TRUE(dest.isWriting());
}

TEST(ReadingTest, reading_chunk_is_dropped_if_target_fifo_isnt_writing) {
    Fifo<20> src;
    Fifo<20> destData;
    ChunkedFifo dest(destData);

    src.write(F("5:abcde"));
    uint8_t length;
    EXPECT_EQ(ReadResult::Valid, src.read(Decimal(&length), F(":"), ChunkWithLength(&length, dest)));
    EXPECT_TRUE(src.isEmpty());
    EXPECT_TRUE(dest.isEmpty());
    EXPECT_FALSE(dest.isWriting());
}

TEST(ReadingTest, hexadecimal_ints_are_read) {
    Fifo<20> fifo;
    fifo.write(F("A"));
    uint8_t uint8;
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(Hexadecimal(&uint8)));
    fifo.write(F("F"));
    EXPECT_EQ(ReadResult::Valid, fifo.read(Hexadecimal(&uint8)));
    EXPECT_EQ(0xAF, uint8);
    EXPECT_TRUE(fifo.isEmpty());

    fifo.write(F("00"));
    EXPECT_EQ(ReadResult::Valid, fifo.read(Hexadecimal(&uint8)));
    EXPECT_EQ(0, uint8);

    fifo.write(F("FF"));
    EXPECT_EQ(ReadResult::Valid, fifo.read(Hexadecimal(&uint8)));
    EXPECT_EQ(0xFF, uint8);

    fifo.write(F("0G"));
    EXPECT_EQ(ReadResult::Invalid, fifo.read(Hexadecimal(&uint8)));
}

TEST(ReadingTest, uint8_t_array_is_read) {
    Fifo<20> fifo;
    fifo.write(F("worl"));
    uint8_t in[5];
    EXPECT_EQ(ReadResult::Incomplete, fifo.read(&in));
    fifo.write('d');
    EXPECT_EQ(ReadResult::Valid, fifo.read(&in));
    EXPECT_EQ('w', in[0]);
    EXPECT_EQ('o', in[1]);
    EXPECT_EQ('r', in[2]);
    EXPECT_EQ('l', in[3]);
    EXPECT_EQ('d', in[4]);
}

struct MockFifo {
    int getReadAvailableCount = 0;

    inline void readStart() {}
    inline void readEnd() {}
    inline void readAbort() {}
    inline bool isReading() { return false; }

    inline int getReadAvailable() {
        getReadAvailableCount++;
        return 2;
    }

    inline void uncheckedRead(uint8_t &target) {}
};

TEST(ReadingTest, reading_fixed_size_args_only_checks_available_bytes_once) {
    MockFifo fifo;
    uint8_t a, b;
    Streams::Impl::read(fifo, &a, &b);
    EXPECT_EQ(1, fifo.getReadAvailableCount);
}

struct MyStruct4 {
    uint8_t a = 0, b = 0;

    typedef Protocol<MyStruct4> P;
    typedef P::Seq<
        P::Binary<uint8_t, &MyStruct4::a>,
        P::Binary<uint8_t, &MyStruct4::b>
    > DefaultProtocol;
};

TEST(ReadingTest, reading_fixed_size_protocol_only_checks_available_bytes_once) {
    MockFifo fifo;
    MyStruct4 s;

    Streams::Impl::read(fifo, &s);
    EXPECT_EQ(1, fifo.getReadAvailableCount);
}
}
