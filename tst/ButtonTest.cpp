#include <gtest/gtest.h>
#include "invoke.hpp"
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
    typedef HAL::Atmel::Int_INT0_ INT;

    bool high = true;
    bool intOn = false;

    bool isHigh() {
        return high;
    }
    void configureAsInputWithPullup() {

    }
    void interruptOnLow() {
        intOn = true;
    }
    void interruptOff() {
        intOn = false;
    }
};

TEST(ButtonTest, button_should_ignore_changes_during_debounce_time) {
    MockRealTimer rt;
    MockPin pin;
    Button<MockPin, MockRealTimer> button(pin, rt);
    EXPECT_EQ(ButtonEvent::UP, button.nextEvent());

    pin.high = false;
    rt.count++;
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = true;
    rt.count++;
    EXPECT_EQ(ButtonEvent::DOWN, button.nextEvent()); // still debouncing

    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());

    pin.high = false;
    rt.count++;
    EXPECT_EQ(ButtonEvent::UP, button.nextEvent()); // still debouncing

    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = true;
    rt.count += uint16_t(toCountsOn(rt, 8_ms));
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());
}

TEST(ButtonTest, button_should_pick_up_button_press_in_interrupt_only) {
    MockRealTimer rt;
    MockPin pin;
    Button<MockPin, MockRealTimer> button(pin, rt);
    EXPECT_TRUE(pin.intOn);

    pin.high = false;
    rt.count++;
    invoke<MockPin::INT>(button);
    EXPECT_FALSE(pin.intOn);
    pin.high = true; // simulate release before invoking nextEvent()

    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());
    EXPECT_TRUE(pin.intOn);
}

TEST(ButtonTest, button_should_pick_up_interrupt_driven_change) {
    MockRealTimer rt;
    MockPin pin;
    Button<MockPin, MockRealTimer> button(pin, rt);
    EXPECT_TRUE(pin.intOn);

    pin.high = false;
    rt.count++;
    invoke<MockPin::INT>(button);
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());
    EXPECT_FALSE(pin.intOn);

    pin.high = true;
    rt.count++;
    EXPECT_EQ(ButtonEvent::DOWN, button.nextEvent()); // still debouncing
    EXPECT_TRUE(pin.intOn); // but interrupt got re-enabled
}
