#include "Serial/PulseCounter.hpp"
#include <gtest/gtest.h>

namespace PulseCounterTest {

using namespace Serial;
using namespace Time;

struct MockComparator {
    typedef uint8_t value_t;

    value_t value = 1;
    value_t target = 0;
    InterruptChain chain;

    MockComparator() {
        std::cout << " hello! " << std::endl;
    }

    void interruptOn() {

    }

    void interruptOff() {

    }

    InterruptChain &interrupt() {
        return chain;
    }

    value_t getValue() {
        return value;
    }

    void setTarget(value_t t) {
        target = t;
    }
};

struct MockPin {
    InterruptChain chain;
    bool high = false;

    InterruptChain &interrupt() {
        return chain;
    }

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
    pin.chain.invoke();
    wasInvoked = false;
    pc.on([] (auto pulse) { wasInvoked = true; });
    EXPECT_FALSE(wasInvoked);

    comp.value = 50;
    pin.chain.invoke();
    pc.on([] (auto pulse) {
        wasInvoked = true;
        EXPECT_EQ(40, pulse.getDuration());
    });
    EXPECT_TRUE(wasInvoked);
}

}
