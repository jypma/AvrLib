#include "Serial/PulseCounter.hpp"
#include <gtest/gtest.h>

namespace PulseCounterTest {

using namespace Serial;
using namespace Time;

struct MockComparator {
    typedef uint8_t value_t;
    typedef HAL::Atmel::InterruptVectors::VectorTIMER0_COMPA_ INT;

    value_t value = 1;
    value_t target = 0;

    MockComparator() {
        std::cout << " hello! " << std::endl;
    }

    void interruptOn() {

    }

    void interruptOff() {

    }

    value_t getValue() {
        return value;
    }

    void setTarget(value_t t) {
        target = t;
    }
};

struct MockPin {
    typedef HAL::Atmel::InterruptVectors::VectorINT0_ INT;
    bool high = false;

    void interruptOff() {

    }

    void interruptOnChange() {

    }

    void configureAsInputWithPullup() {

    }

    bool isHigh() {
        return high;
    }
};


bool wasInvoked;
TEST(PulseCounterTest, pulsecounter_ignores_pulses_shorter_than_minimum_length) {
    MockComparator comp;
    MockPin pin;

    auto pc = pulseCounter<64>(comp, pin, 20_counts);

    comp.value = 10; // less than 20
    decltype(pc)::onPinChangedHandler::invoke(pc);
    wasInvoked = false;
    pc.on([] (auto pulse) { wasInvoked = true; });
    EXPECT_FALSE(wasInvoked);

    comp.value = 50;
    decltype(pc)::onPinChangedHandler::invoke(pc);
    pc.on([] (auto pulse) {
        wasInvoked = true;
        EXPECT_EQ(40, pulse.getDuration());
    });
    EXPECT_TRUE(wasInvoked);
}

}
