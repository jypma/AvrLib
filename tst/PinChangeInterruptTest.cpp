#include "HAL/Atmel/PinChangeInterrupt.hpp"
#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include <gtest/gtest.h>

namespace PinChangeInterruptTest {

using namespace HAL::Atmel;
using namespace HAL::Atmel::InterruptHandlers;

struct MockPCINTInfo {
	typedef Int_PCINT0_ PCINT;
	static auto constexpr &PIN = PINC;
	static auto constexpr &PCMSK = PCMSK0;
	static auto constexpr &PCIE = PCIE0;
};

TEST(PinChangeInterruptTest, multiple_handlers_on_same_PCINT_are_invoked_for_their_bitmask) {
	PinChangeInterrupt<MockPCINTInfo, (1 << 0)> intForBit0;
	PinChangeInterrupt<MockPCINTInfo, (1 << 1)> intForBit1;
	PinChangeInterrupt<MockPCINTInfo, (1 << 2)> intForBit2;

	PINC.apply(~(PINC0 | PINC1 | PINC2 | PINC3 | PINC4 | PINC5 | PINC6));

	intForBit0.interruptOnChange();
	intForBit1.interruptOnChange();
	intForBit2.interruptOnChange();

	PINC0.set();

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

	PINC1.set();

	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVector<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC0.clear();
	PINC1.clear();

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

    PINC.apply(~(PINC0 | PINC1 | PINC2 | PINC3 | PINC4 | PINC5 | PINC6));

	intForBit0.interruptOnChange();
	intForBit1.interruptOnChange();
	intForBit2.interruptOnChange();

	PINC0.set();

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

	PINC1.set();

	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 0)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 1)>::wrap([&] { invoked = true; });
	EXPECT_TRUE(invoked);
	invoked = false;
	PinChangeVectorOnChange<MockPCINTInfo, (1 << 2)>::wrap([&] { invoked = true; });
	EXPECT_FALSE(invoked);

	PINC0.clear();
	PINC1.clear();

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
    PINC.apply(~(PINC0 | PINC1 | PINC2 | PINC3 | PINC4 | PINC5 | PINC6));
    PCINT0_vect();
    PINC0.set();
    PCINT0_vect();
    EXPECT_EQ(1, app.changes);
}

}
