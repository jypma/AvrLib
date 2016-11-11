#include "Serial/PulseCounter.hpp"
#include <gtest/gtest.h>
#include "Mocks.hpp"
#include "invoke.hpp"

namespace PulseCounterTest {

using namespace Serial;
using namespace Time;
using Mocks::MockPin;

TEST(PulseCounterTest, pulsecounter_reacts_to_changes_only_after_minimum_length) {
	typedef Mocks::MockComparator<uint8_t, 10> MockComparator;
    MockComparator comp;
    MockPin pin;
    pin.high = false;

    auto pc = pulseCounter<64>(comp, pin, 20_counts);
    EXPECT_FALSE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(20, comp.target);

    // first transition: longer than minimum length: comparator fires first, with pin unchanged. Pin change fires afterwards.
    comp.value = 20;
    invoke<MockComparator::INT>(pc);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(20, comp.target);

    comp.value = 24;
    pin.high = true;
    invoke<MockPin::INT>(pc);
    EXPECT_FALSE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(44, comp.target);

    bool wasInvoked = false;
    pc.on([&] (auto pulse) {
        wasInvoked = true;
        EXPECT_EQ(24, pulse.getDuration());
    });
    EXPECT_TRUE(wasInvoked);
    wasInvoked = false;
}

TEST(PulseCounterTest, pulsecounter_works_with_16_bit_timers) {
	typedef Mocks::MockComparator<uint16_t, 10> MockComparator;
    MockComparator comp;
    MockPin pin;
    pin.high = false;

    auto pc = pulseCounter<64>(comp, pin);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(65535, comp.target);

    comp.value = 20;
    pin.high =  true;
    invoke<MockPin::INT>(pc);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(19, comp.target);

    comp.value = 16000;
    pin.high = false;
    invoke<MockPin::INT>(pc);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_TRUE(comp.isInterruptOn);
    EXPECT_EQ(15999, comp.target);

    // Simulate timed out
    comp.value = 15999;
    invoke<MockComparator::INT>(pc);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_FALSE(comp.isInterruptOn);

    bool wasInvoked = false;
    pc.on([&] (auto pulse) {
        wasInvoked = true;
        EXPECT_FALSE(pulse.isHigh());
        EXPECT_EQ(20, pulse.getDuration());
    });
    EXPECT_TRUE(wasInvoked);
    wasInvoked = false;
    pc.on([&] (auto pulse) {
        wasInvoked = true;
        EXPECT_TRUE(pulse.isHigh());
        EXPECT_EQ(15980, pulse.getDuration());
    });
    EXPECT_TRUE(wasInvoked);
}

}
