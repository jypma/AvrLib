#include "gtest/gtest.h"
#include "Serial/RS232Tx.hpp"
#include "Mocks.hpp"
#include "invoke.hpp"

namespace RS232Txtest {

using namespace Mocks;
using namespace Serial;

TEST(RS232Tx, should_send_until_done) {
	MockPinOnComparator<uint16_t> pin;
	Fifo<32> data;
	auto rs = Serial::Impl::RS232Tx<MockPinOnComparator<uint16_t>,57600>(pin, data);
	rs.write(FB(0,1));

	// 4 interrupts for bit 0
	for (int i = 0; i < 4; i++) {
		pin.comparator.advanceToTargetAndInvoke(rs);
	}

	// 5 interrupts for bit 1
	for (int i = 0; i < 5; i++) {
		pin.comparator.advanceToTargetAndInvoke(rs);
	}

	EXPECT_EQ(0, data.getSize());
}

TEST(RS232Tx, should_send_0x00_out) {
	MockPinOnComparator<uint8_t> pin;
	Fifo<32> data;
	auto rs = Serial::Impl::RS232Tx<MockPinOnComparator<uint8_t>,57600>(pin, data);
	std::cout << int(rs.bitLength) << std::endl;
	rs.write(FB(0)); 	// expected: 0 | 0 0 0 0 0 0 0 0 | 1

	// 0 | 0 0 0
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(136, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 0 0 0 0
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(16, pin.comparator.target); // 136 + 136 - 256
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 0
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(50, pin.comparator.target); // 16 + 34
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// | 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(84, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_FALSE(pin.comparator.isInterruptOn);
}

TEST(RS232Tx, should_send_0x01_out) {
	MockPinOnComparator<uint8_t> pin;
	Fifo<32> data;
	auto rs = Serial::Impl::RS232Tx<MockPinOnComparator<uint8_t>,57600>(pin, data);
	std::cout << int(rs.bitLength) << std::endl;
	rs.write(FB(1)); 	// expected: 0 | 1 0 0 0 0 0 0 0 | 1

	// 0 |
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(34, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(68, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 0 0 0 0
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(204, pin.comparator.target); // 68 + 136
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 0 0 0
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(50, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// | 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(84, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_FALSE(pin.comparator.isInterruptOn);
}

TEST(RS232Tx, should_send_0x80_out) {
	MockPinOnComparator<uint8_t> pin;
	Fifo<32> data;
	auto rs = Serial::Impl::RS232Tx<MockPinOnComparator<uint8_t>,57600>(pin, data);
	std::cout << int(rs.bitLength) << std::endl;
	rs.write(FB(0x80)); 	// expected: 0 | 0 0 0 0 0 0 0 1 | 1

	// 0 | 0 0 0
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(136, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 0 0 0 0
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(16, pin.comparator.target); // 136 + 136 - 256
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 1 | 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(84, pin.comparator.target); // 16 + 34
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_FALSE(pin.comparator.isInterruptOn);
}

TEST(RS232Tx, should_send_0xFF_out) {
	MockPinOnComparator<uint8_t> pin;
	Fifo<32> data;
	auto rs = Serial::Impl::RS232Tx<MockPinOnComparator<uint8_t>,57600>(pin, data);
	std::cout << int(rs.bitLength) << std::endl;
	rs.write(FB(0xFF)); 	// expected: 0 | 1 1 1 1 1 1 1 1 | 1

	// 0 |
	EXPECT_FALSE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::toggle_on_match, pin.comparator.mode);
	EXPECT_EQ(34, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 1 1 1 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(170, pin.comparator.target); // 16 + 34
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 1 1 1 1 |
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(50, pin.comparator.target); // 16 + 34
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	// 1
	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comparator.mode);
	EXPECT_EQ(84, pin.comparator.target);
	EXPECT_TRUE(pin.comparator.isInterruptOn);

	pin.comparator.advanceToTargetAndInvoke(rs);
	EXPECT_TRUE(pin.high);
	EXPECT_FALSE(pin.comparator.isInterruptOn);
}

TEST(RS232Tx, should_send_0xCC_out) {

}

TEST(RS232Tx, should_send_0x55_out) {

}

}
