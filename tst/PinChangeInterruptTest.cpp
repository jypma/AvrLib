#include "HAL/Atmel/PinChangeInterrupt.hpp"
#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include <gtest/gtest.h>

namespace PinChangeInterruptTest {

using namespace HAL::Atmel;
using namespace HAL::Atmel::InterruptHandlers;

struct MockPCINTInfo {
	typedef Int_PCINT0_ PCINT;
	static auto constexpr pin = &PINC;
	static auto constexpr pcmsk = &PCMSK0;
	static auto constexpr PCIE = PCIE0;
};

TEST(PinChangeInterruptTest, multiple_handlers_on_same_PCINT_are_invoked_for_their_bitmask) {
	PinChangeInterrupt<MockPCINTInfo, (1 << 0)> intForBit0;
	PinChangeInterrupt<MockPCINTInfo, (1 << 1)> intForBit1;
	PinChangeInterrupt<MockPCINTInfo, (1 << 2)> intForBit2;

	PINC = 0;

	intForBit0.interruptOnChange();
	intForBit1.interruptOnChange();
	intForBit2.interruptOnChange();

	PINC = (1 << 0);

	bool invoked;
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC = (1 << 0) | (1 << 1);

	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC = 0;

	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
}

TEST(PinChangeInterruptOnChangeTest, multiple_handlers_on_same_PCINT_are_invoked_for_their_bitmask) {
	PinChangeInterruptOnChange<MockPCINTInfo, (1 << 0)> intForBit0;
	PinChangeInterruptOnChange<MockPCINTInfo, (1 << 1)> intForBit1;
	PinChangeInterruptOnChange<MockPCINTInfo, (1 << 2)> intForBit2;

	PINC = 0;

	intForBit0.interruptOnChange();
	intForBit1.interruptOnChange();
	intForBit2.interruptOnChange();

	PINC = (1 << 0);

	bool invoked;
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC = (1 << 0) | (1 << 1);

	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC = 0;

	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
}

#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, USART_RX_, USART_UDRE_, PCINT0_, INT0_)

struct TheApp {
	uint8_t changes;
	void onChange() { changes++; }
	typedef On<TheApp, PinChangeVectorOnChange<MockPCINTInfo, (1 << 0)>, &TheApp::onChange> Handlers;
};

TheApp app;
mkISRS

TEST(PinChangeInterruptOnChangeTest, binds_to_interrupt_when_used_as_handler) {
    app.changes = 0;
    PINC = 0;
    PCINT0_vect();
    PINC = 1;
    PCINT0_vect();
    EXPECT_EQ(1, app.changes);
}

}
