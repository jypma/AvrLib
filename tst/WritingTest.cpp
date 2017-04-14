#include <gtest/gtest.h>
#include "Option.hpp"
#include "Fifo.hpp"
#include "ChunkedFifo.hpp"
#include "EEPROMTest.hpp"
#include "Streams/WritingTypes.hpp"

namespace WritingTest {

using namespace Streams;

struct MockFifo {
    uint8_t fullCount = 0;
    uint8_t writeInvocations = 0;
    uint8_t written = 0;
    int getSpaceCount = 0;
    bool writing = false;

    bool isFull() {
        if (fullCount > 0) {
            fullCount--;
            return true;
        } else {
            return false;
        }
    }

    bool hasSpace() {
        getSpaceCount++;
        return !isFull();
    }

    void uncheckedWrite(uint8_t value) {
        writeInvocations++;
        written = value;
    }

    inline int getSpace() {
        getSpaceCount++;
        return 2;
    }

    void writeStart() {
        writing = true;
    }

    void writeEnd() {
        writing = false;
    }

    void writeAbort() {
        writing = false;
    }

    bool isWriting() {
        return writing;
    }
};

TEST(WritingTest, blocking_write_semantics_write_each_byte_separately_after_waiting) {
    MockFifo fifo;
    sei();
    EXPECT_TRUE(Impl::BlockingWriteSemantics<MockFifo>::canWrite(fifo, 255));
    fifo.fullCount = 2;
    Impl::BlockingWriteSemantics<MockFifo>::write(fifo, 42);
    EXPECT_EQ(1, fifo.writeInvocations);
    EXPECT_EQ(42, fifo.written);
}

TEST(WritingTest, blocking_write_semantics_does_not_block_when_interrupts_are_off) {
    MockFifo fifo;
    cli();
    fifo.fullCount = 1;
    Impl::BlockingWriteSemantics<MockFifo>::write(fifo, 42);
    EXPECT_EQ(0, fifo.writeInvocations);

    fifo.fullCount = 0;
    Impl::BlockingWriteSemantics<MockFifo>::write(fifo, 42);
    EXPECT_EQ(1, fifo.writeInvocations);
    EXPECT_EQ(42, fifo.written);
    sei();
}

TEST(WritingTest, can_write_little_endian_ints) {
    Fifo<42> fifo;
    EXPECT_TRUE(fifo.write(uint8_t(214), int8_t(-16), uint16_t(214), int16_t(-16), uint32_t(214), int32_t(-16)));
    uint8_t b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13;
    fifo.read(&b0,&b1,&b2,&b3,&b4,&b5,&b6,&b7,&b8,&b9,&b10,&b11,&b12,&b13);
    EXPECT_EQ(214, b0);

    EXPECT_EQ(240, b1);

    EXPECT_EQ(214, b2);
    EXPECT_EQ(0, b3);

    EXPECT_EQ(240, b4);
    EXPECT_EQ(255, b5);

    EXPECT_EQ(214, b6);
    EXPECT_EQ(0, b7);
    EXPECT_EQ(0, b8);
    EXPECT_EQ(0, b9);

    EXPECT_EQ(240, b10);
    EXPECT_EQ(255, b11);
    EXPECT_EQ(255, b12);
    EXPECT_EQ(255, b13);
}

TEST(WritingTest, write_that_does_not_fit_aborts_fully) {
    Fifo<13> fifo;
    EXPECT_FALSE(fifo.write(uint8_t(214), int8_t(-42), uint16_t(214), int16_t(-42), uint32_t(214), int32_t(-42)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_int8) {
    Fifo<32> fifo;
    fifo.write(dec(int8_t(0)), dec(int8_t(-1)), dec(int8_t(127)), dec(int8_t(-128)));
    EXPECT_TRUE(fifo.read(F("0-1127-128")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_uint8) {
    Fifo<32> fifo;
    fifo.write(dec(int8_t(0)), dec(uint8_t(106)), dec(uint8_t(10)), dec(uint8_t(255)));
    EXPECT_TRUE(fifo.read(F("010610255")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_uint16) {
    Fifo<32> fifo;
    fifo.write(dec(uint16_t(0)), dec(uint16_t(65535)), dec(uint16_t(256)));
    EXPECT_TRUE(fifo.read(F("065535256")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_int16) {
    Fifo<32> fifo;
    fifo.write(dec(int16_t(0)), dec(int16_t(32767)), dec(int16_t(-32768)));
    EXPECT_TRUE(fifo.read(F("032767-32768")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_uint32) {
    Fifo<32> fifo;
    fifo.write(dec(uint32_t(0)), dec(uint32_t(4002)), dec(uint32_t(4294967295)));
    EXPECT_TRUE(fifo.read(F("040024294967295")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_decimal_int32) {
    Fifo<32> fifo;
    fifo.write(dec(int32_t(0)), dec(int32_t(2147483647)), dec(int32_t(-2147483648)));
    EXPECT_TRUE(fifo.read(F("02147483647-2147483648")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_nested) {
    Fifo<32> fifo;
    uint8_t a = 1;
    uint8_t b = 2;

    auto doIt = Nested([&] (auto write) {
        if (write(a)) {
            if (a > 0) {
                return write(b);
            } else {
                return true;
            }
        } else {
            return false;
        }
    });

    fifo.write(doIt);
    EXPECT_TRUE(fifo.read(FB(1,2)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_flash_string) {
    Fifo<8> fifo;
    EXPECT_FALSE(fifo.write(F("123456789")));
    EXPECT_TRUE(fifo.write(F("12345678")));
    EXPECT_EQ(8, fifo.getSize());
    EXPECT_TRUE(fifo.read(F("12345678")));
    EXPECT_TRUE(fifo.isEmpty());

    EXPECT_TRUE(fifo.write(F("1")));
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_TRUE(fifo.read(F("1")));
    EXPECT_TRUE(fifo.isEmpty());

}

TEST(WritingTest, can_write_padding) {
    Fifo<8> fifo;
    EXPECT_TRUE(fifo.write(Padding(0)));
    EXPECT_TRUE(fifo.isEmpty());

    EXPECT_TRUE(fifo.write(Padding(1)));
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(0)));
    EXPECT_TRUE(fifo.isEmpty());

    EXPECT_TRUE(fifo.write(Padding(2)));
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(0,0)));
    EXPECT_TRUE(fifo.isEmpty());

    EXPECT_TRUE(fifo.write(Padding(8)));
    EXPECT_EQ(8, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(0,0,0,0,0,0,0,0)));
    EXPECT_TRUE(fifo.isEmpty());

    EXPECT_FALSE(fifo.write(Padding(9)));
    EXPECT_TRUE(fifo.isEmpty());
}

struct Struct1 {
    uint8_t a;
    uint8_t b;
    Struct1(uint8_t _a, uint8_t _b): a(_a), b(_b) {}

    typedef Protocol<Struct1> P;
    typedef P::Seq<
        P::Binary<uint8_t, &Struct1::a>,
        P::Binary<uint8_t, &Struct1::b>
    > DefaultProtocol;

    bool hasA() const { return a > 0; }

    typedef P::Seq<
        P::Binary<uint8_t, &Struct1::a>,
        P::Conditional<&Struct1::hasA,
            P::Binary<uint8_t, &Struct1::b>
        >
    > AlternateProto;
};

TEST(WritingTest, can_write_struct_with_default_protocol) {
    Fifo<8> fifo;
    Struct1 s = {1, 2};
    fifo.write(&s);
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(1,2)));
    EXPECT_TRUE(fifo.isEmpty());

    const Struct1 cs = {1, 2};
    fifo.write(&cs);
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(1,2)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_struct_with_specific_protocol) {
    Fifo<8> fifo;
    Struct1 s = {42, 2};
    typedef Protocol<Struct1> P;
    typedef P::Seq<
        P::Binary<uint8_t, &Struct1::a>,
        P::Decimal<uint8_t, &Struct1::a>,
        P::Hexadecimal<uint8_t, &Struct1::a>
    > CustomProto;

    fifo.write(as<CustomProto>(&s));
    EXPECT_EQ(5, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(42)));
    EXPECT_TRUE(fifo.read(F("42")));
    EXPECT_TRUE(fifo.read(F("2A")));
    EXPECT_TRUE(fifo.isEmpty());

    const Struct1 cs = {42, 2};
    fifo.write(as<CustomProto>(&cs));
    EXPECT_EQ(5, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(42)));
    EXPECT_TRUE(fifo.read(F("42")));
    EXPECT_TRUE(fifo.read(F("2A")));
    EXPECT_TRUE(fifo.isEmpty());
}

struct Struct2 {
    Struct1 s1;

    typedef Protocol<Struct2> P;
    typedef P::Seq<
        P::Object<Struct1, &Struct2::s1>
    > DefaultProtocol;
};

TEST(WritingTest, can_write_const_struct_with_nested_struct_of_default_protocol) {
    Fifo<8> fifo;
    const Struct2 s = { {1, 2} };
    fifo.write(&s);
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(1,2)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_const_struct_with_conditional_fields_in_protocol) {
    Fifo<8> fifo;
    const Struct1 s1 = {1, 2};
    typedef Protocol<Struct1> P;
    typedef P::Seq<
        P::Binary<uint8_t, &Struct1::a>,
        P::Conditional<&Struct1::hasA,
            P::Binary<uint8_t, &Struct1::b>
        >
    > CustomProto;

    fifo.write(as<CustomProto>(&s1));
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(1,2)));
    EXPECT_TRUE(fifo.isEmpty());

    const Struct1 s2 = {0, 2};
    fifo.write(as<CustomProto>(&s2));
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_TRUE(fifo.read(FB(0)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_binary_from_eeprom) {
    Fifo<16> fifo;
    eeprom_set(&EEPROM::data, 214);
    eeprom_set(&EEPROM::remotePort, 42);
    eeprom_set(&EEPROM::name, "hello");

    fifo.write(&EEPROM::data, &EEPROM::remotePort, dec(&EEPROM::remotePort), &EEPROM::name);

    EXPECT_TRUE(fifo.read(FB(214, 42, 0)));
    EXPECT_TRUE(fifo.read(F("42hello")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, writes_null_terminated_eeprom_string) {
    eeprom_contents[1] = 42;
    eeprom_contents[2] = 0;

    Fifo<200> f;
    f.write(&EEPROM::name);

    EXPECT_EQ(1, f.getSize());
    EXPECT_EQ(42, f.peek());
}

TEST(WritingTest, doesnt_read_past_end_of_eeprom_string_when_null_terminator_is_missing) {
    eeprom_contents[1] = 42;
    eeprom_contents[2] = 42;
    eeprom_contents[3] = 42;
    eeprom_contents[4] = 42;
    eeprom_contents[5] = 42;
    eeprom_contents[6] = 42;
    eeprom_contents[7] = 42;
    eeprom_contents[8] = 42;
    eeprom_contents[9] = 42;
    eeprom_contents[10] = 42;

    Fifo<200> f;
    f.write(&EEPROM::name);

    EXPECT_EQ(10, f.getSize());
}

TEST(WritingTest, can_write_string_from_eeprom) {
    Fifo<8> fifo;
    eeprom_set(&EEPROM::name, "");
    fifo.write(&EEPROM::name);
    EXPECT_TRUE(fifo.isEmpty());

    eeprom_set(&EEPROM::name, "foo");
    fifo.write(&EEPROM::name);
    EXPECT_TRUE(fifo.read(F("foo")));
    EXPECT_TRUE(fifo.isEmpty());

    eeprom_set(&EEPROM::name, "123456789"); // FIFO has only space for 8
    fifo.write(&EEPROM::name);
    EXPECT_TRUE(fifo.isEmpty());

    eeprom_set(&EEPROM::name, "12345678");
    fifo.write(&EEPROM::name);
    EXPECT_TRUE(fifo.read(F("12345678")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_chunk_from_chunked_fifo_that_is_open_for_reading) {
    Fifo<200> inputData;
    ChunkedFifo input(inputData);
    input.write(F("hello"));

    Fifo<20> fifo;
    input.readStart();
    fifo.write(input);
    EXPECT_TRUE(fifo.read(F("hello")));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_enum_and_chunk) {
    Fifo<24> inputData;
    ChunkedFifo input(inputData);
    input.write(F("1234567890123456789"));

    Fifo<64> outputData;
    ChunkedFifo output(outputData);
    input.readStart();
    uint8_t value = 'H';
    EXPECT_TRUE(output.write(value, input));
    EXPECT_TRUE(output.read(F("H1234567890123456789")));
    EXPECT_TRUE(output.isEmpty());
}

TEST(WritingTest, write_from_empty_chunked_fifo_is_ignored) {
    Fifo<200> inputData;
    ChunkedFifo input(inputData);

    Fifo<20> fifo;
    fifo.write(input);
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, write_from_chunked_fifo_thats_not_reading_is_ignored) {
    Fifo<200> inputData;
    ChunkedFifo input(inputData);
    input.write(F("hello"));

    Fifo<20> fifo;
    fifo.write(input);
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_FALSE(input.isReading());
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, write_rolls_back_on_full_fifo) {
    Fifo<8> fifo;
    fifo.write(F("hello"));
    fifo.write(F("world"));
    EXPECT_EQ(5, fifo.getSize());
    EXPECT_FALSE(fifo.isWriting());
}

TEST(WritingTest, blocking_writes_do_not_occur_in_an_atomic_block) {
    // since that would defeat the purpose of blocking. Unless, if needed later, we turn
    // interrupts on while inside the block() function.
    Fifo<8> fifo;
    sei();
    bool intWasEnabled = false;
    fifo.write(Nested([&] (auto write) { intWasEnabled = (SREG & (1 << SREG_I)) > 0; return true; }));
    EXPECT_TRUE(intWasEnabled);
}

TEST(WritingTest, writing_constant_size_args_only_checks_size_once) {
    MockFifo fifo;
    Streams::Impl::writeIfSpace(fifo, uint8_t(42), uint8_t(84));
    EXPECT_EQ(1, fifo.getSpaceCount);
}

TEST(WritingTest, can_write_uint8_t_array) {
    Fifo<8> fifo;
    uint8_t out[5] = {1, 2, 3, 4, 5};
    fifo.write(&out);
    EXPECT_TRUE(fifo.read(FB(1,2,3,4,5)));
}

TEST(WritingTest, can_write_decimal_delegate_fifo) {
    Fifo<16> fifo;
    Fifo<16> input;
    input.write(FB(1,2,3));
    auto in = input.in();
    in.readStart();
    EXPECT_TRUE(fifo.write(dec(&in)));
    EXPECT_TRUE(fifo.read(F("1,2,3")));
    EXPECT_TRUE(fifo.isEmpty());
}


TEST(WritingTest, doe_not_write_decimal_delegate_fifo_if_no_space) {
    Fifo<4> fifo;
    Fifo<16> input;
    input.write(FB(1,2,3));
    auto in = input.in();
    in.readStart();
    EXPECT_FALSE(fifo.write(dec(&in)));
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_TRUE(in.isReading());
    input.readAbort();

    EXPECT_TRUE(fifo.write('a')); // write 1 byte, to align failure with "2" instead of ",".
    input.readStart();
    EXPECT_FALSE(fifo.write(dec(&input)));
    EXPECT_EQ(1, fifo.getSize());
}

TEST(WritingTest, does_not_write_decimal_delegate_fifo_that_isnt_reading) {
    Fifo<16> fifo;
    Fifo<16> input;
    input.write(FB(1,2,3));
    auto in = input.in();
    EXPECT_TRUE(fifo.write(dec(&in)));
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(WritingTest, can_write_literal_strings) {
    Fifo<16> fifo;
    EXPECT_TRUE(fifo.write("hello"));
    EXPECT_EQ(5, fifo.getSize());
    EXPECT_TRUE(fifo.read(F("hello")));
}

TEST(WritingTest, can_skip_literal_strings_if_no_space) {
    Fifo<4> fifo;
    EXPECT_FALSE(fifo.write("hello"));
    EXPECT_EQ(0, fifo.getSize());
}

struct Struct1Sub: public Struct1 {
    Struct1Sub(uint8_t a, uint8_t b): Struct1(a,b) {}
};

TEST(WritingTest, can_write_subclass_of_struct_with_specific_proto_in_parent) {
    Fifo<8> fifo;
    Struct1Sub s = {1, 2};
    fifo.write(as<Struct1::AlternateProto>(&s));
    EXPECT_EQ(2, fifo.getSize());
}

TEST(WritingTest, can_write_protobuf) {
	using namespace Protobuf;
	Fifo<16> fifo;
	uint32_t i32 = 1;
	fifo.write(Varint<1>(i32));
	EXPECT_EQ(2, fifo.getSize());
	EXPECT_TRUE(fifo.read(FB(8,1)));

	fifo.clear();
	i32 = 0xFFFFFFFF;
	fifo.write(Varint<1>(i32));
	EXPECT_EQ(6, fifo.getSize());
	EXPECT_TRUE(fifo.read(FB(8,255,255,255,255,15)));

	fifo.clear();
	int8_t i8 = -1;
	fifo.write(Varint<1>(i8));
	EXPECT_EQ(2, fifo.getSize());
	EXPECT_TRUE(fifo.read(FB(8,1)));

	fifo.clear();
	i8 = 1;
	fifo.write(Varint<1>(i8));
	EXPECT_EQ(2, fifo.getSize());
	EXPECT_TRUE(fifo.read(FB(8,2)));
}

struct MyPBStruct {
    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;

    Option<uint8_t> optA;
    Option<uint16_t> optB;

    typedef Protobuf::Protocol<MyPBStruct> P;

    typedef P::Message<
		P::Varint<1, uint8_t, &MyPBStruct::uint8>,
		P::Varint<2, uint16_t, &MyPBStruct::uint16>,
		P::Varint<3, uint32_t, &MyPBStruct::uint32>,
	    P::Varint<4, Option<uint8_t>, &MyPBStruct::optA>,
		P::Varint<5, Option<uint16_t>, &MyPBStruct::optB>
	> DefaultProtocol;
};

TEST(WritingTest, can_write_protobuf_struct) {
	Fifo<16> fifo;
	MyPBStruct s;
	s.uint8 = 42;
	s.uint16 = 0xFEDB;
	s.uint32 = 0xFEDBFEDB;
	fifo.write(&s);
	const auto expected = FB(8,42,16,219,253,3,24,219,253,239,246,15);
	EXPECT_TRUE(fifo.read(expected));

	fifo.write(expected);
	s.uint8 = 0;
	s.uint16 = 0;
	s.uint32 = 0;
	EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
	EXPECT_EQ(42, s.uint8);
	EXPECT_EQ(0xFEDB, s.uint16);
	EXPECT_EQ(0xFEDBFEDB, s.uint32);
}

TEST(WritingTest, can_write_protobuf_struct_with_optional) {
	Fifo<24> fifo;
	MyPBStruct s;
	s.uint8 = 42;
	s.uint16 = 0xFEDB;
	s.uint32 = 0xFEDBFEDB;
	s.optA = 40;
	s.optB = 42;
	fifo.write(&s);
	const auto expected = FB(8,42,16,219,253,3,24,219,253,239,246,15,4 << 3,40,5 << 3,42);
	EXPECT_TRUE(fifo.read(expected));

	fifo.write(expected);
	s.uint8 = 0;
	s.uint16 = 0;
	s.uint32 = 0;
	s.optA = none();
	EXPECT_EQ(ReadResult::Valid, fifo.read(&s));
	EXPECT_EQ(42, s.uint8);
	EXPECT_EQ(0xFEDB, s.uint16);
	EXPECT_EQ(0xFEDBFEDB, s.uint32);
	EXPECT_EQ(some(40), s.optA);
	EXPECT_EQ(some(42), s.optB);
}

TEST(WritingTest, can_write_optional_decimal) {
	Option<uint8_t> present = 42;
	Option<uint8_t> absent = none();
	Fifo<24> fifo;

	fifo.write(F("This is not written"), dec(absent));
	EXPECT_TRUE(fifo.isEmpty());
	fifo.write(F("This is written"), dec(present));
	EXPECT_TRUE(fifo.read(F("This is written42")));
}

struct Measurement {
	Option<int16_t> temp;
	Option<uint16_t> humidity;
	uint16_t supply;
	uint8_t seq;
	uint16_t sender;
	Option<uint16_t> lux;
	Option<uint8_t> motion;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, uint16_t, &Measurement::supply>,
		P::Varint<10, Option<int16_t>, &Measurement::temp>,
		P::Varint<11, Option<uint16_t>, &Measurement::humidity>,
		P::Varint<12, Option<uint16_t>, &Measurement::lux>,
	    P::Varint<13, Option<uint8_t>, &Measurement::motion>
	> DefaultProtocol;
};

TEST(WritingTest, can_write_opt_protobuf_to_chunked_fifo) {
    Fifo<64> data;
    ChunkedFifo fifo(data);

    EXPECT_EQ(0, fifo.getSize());
    EXPECT_EQ(0, data.getSize());

    Measurement m;
	m.temp = 239;
	m.humidity = 427;
	m.supply = 4788;
	m.seq = 3;
    m.sender = 'Q' << 8 | '1';
    m.lux = 4693;
    fifo.write(uint16_t(0), uint8_t(42), &m);

    EXPECT_EQ(22, fifo.getSize());
    EXPECT_EQ(22, data.getSize());
}

TEST(WritingTest, can_zigzag_signed_ints) {
    Fifo<64> fifo;
    int16_t i = -1024;
	fifo.write(Protobuf::Varint<1>(i));
	EXPECT_TRUE(fifo.read(FB(8,255,15)));
}

struct SignedInt {
	int16_t sint;

    typedef Protobuf::Protocol<SignedInt> P;
    typedef P::Message<
		P::Varint<1, int16_t, &SignedInt::sint>
    > DefaultProtocol;
};

TEST(WritingTest, can_read_back_signed_ints) {
    Fifo<64> fifo;
    SignedInt s = { -1024 };
	fifo.write(&s);
	SignedInt s2;
	EXPECT_TRUE(fifo.read(&s2));
	EXPECT_EQ(-1024, s2.sint);
}

}
