#include <gtest/gtest.h>
#include "Mechanic/Button.hpp"

using namespace Mechanic;

struct MockRealTimer {
    typedef uint16_t value_t;
    static constexpr uint64_t maximum = 65535;
    static constexpr uint8_t prescalerPower2 = 8;

    uint32_t count = 0;
    uint32_t tick = 0;
    uint32_t counts() const {
        return count;
    }

    uint32_t ticks() const {
        return tick;
    }
};

struct MockPin {
    bool high;

    bool isHigh() {
        return high;
    }
    void configureAsInputWithPullup() {

    }
};

TEST(ButtonTest, button_should_ignore_changes_during_debounce_time) {
    MockRealTimer rt;
    MockPin pin;
    Button<MockPin, MockRealTimer> button(pin, rt);

    pin.high = true;
    rt.count++;
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = false;
    rt.count++;
    EXPECT_EQ(ButtonEvent::IDLE, button.nextEvent());

    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());

    pin.high = true;
    rt.count++;
    EXPECT_EQ(ButtonEvent::IDLE, button.nextEvent());

    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = false;
    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());
}
