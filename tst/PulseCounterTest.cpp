#include "Serial/PulseCounter.hpp"
#include <gtest/gtest.h>
#include "invoke.hpp"

namespace PulseCounterTest {

using namespace Serial;
using namespace Time;

struct MockComparator {
    typedef uint8_t value_t;
    typedef HAL::Atmel::Int_TIMER0_COMPA_ INT;

    static constexpr uint8_t prescalerPower2 = 10;

    value_t value = 0;
    value_t target = 0;
    bool isInterruptOn = false;

    MockComparator() {
        std::cout << " hello! " << std::endl;
    }

    void interruptOn() {
        isInterruptOn = true;
    }

    void interruptOff() {
        isInterruptOn = false;
    }

    value_t getValue() {
        return value;
    }

    void setTarget(value_t t) {
        target = t;
    }
};

struct MockPin {
    typedef HAL::Atmel::Int_INT0_ INT;
    bool high = false;
    bool isInterruptOn = false;

    void interruptOff() {
        isInterruptOn = false;
    }

    void interruptOnChange() {
        isInterruptOn = true;
    }

    void configureAsInputWithPullup() {

    }

    void configureAsOutput() {

    }

    bool isHigh() {
        return high;
    }

    void setHigh(bool h) {
        high = h;
    }

    void setLow() {
        high = false;
    }
};


TEST(PulseCounterTest, pulsecounter_reacts_to_changes_only_after_minimum_length) {
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
	FAIL("pending");
}

}
