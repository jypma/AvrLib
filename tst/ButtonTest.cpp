#include <gtest/gtest.h>
#include "invoke.hpp"
#include "Mechanic/Button.hpp"
#include "Mocks.hpp"

namespace ButtonTest {

using namespace Mechanic;
using namespace Mocks;

TEST(ButtonTest, button_should_ignore_changes_during_debounce_time) {
    MockRealTimer rt;
    MockPin pin;
    pin.high = true; // has pull-up
    auto button = Button(rt, pin);
    EXPECT_FALSE(pin.isOutput);
    EXPECT_TRUE(pin.isInterruptOn);
    EXPECT_EQ(ButtonEvent::UP, button.nextEvent());

    pin.high = false;
    rt.advance(1_counts);
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = true;
    rt.advance(1_counts);
    EXPECT_EQ(ButtonEvent::DOWN, button.nextEvent()); // still debouncing

    rt.advance(8_ms);
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());

    pin.high = false;
    rt.advance(1_counts);
    EXPECT_EQ(ButtonEvent::UP, button.nextEvent()); // still debouncing

    rt.advance(8_ms);
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());

    pin.high = true;
    rt.advance(8_ms);
    EXPECT_EQ(ButtonEvent::RELEASED, button.nextEvent());
}

TEST(ButtonTest, button_should_pick_up_button_press_released_before_loop) {
    MockRealTimer rt;
    MockPin pin;
    pin.high = true; // has pull-up
    auto button = Button(rt, pin);

    pin.high = false;
    rt.advance(1_counts);
    invoke<MockPin::INT>(button);
    EXPECT_TRUE(pin.isInterruptOn);
    pin.high = true; // simulate release before invoking nextEvent()

    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());
}

TEST(ButtonTest, button_should_pick_up_interrupt_driven_change) {
    MockRealTimer rt;
    MockPin pin;
    pin.high = true; // has pull-up
    auto button = Button(rt, pin);
    EXPECT_TRUE(pin.isInterruptOn);

    pin.high = false;
    rt.advance(1_counts);
    invoke<MockPin::INT>(button);
    EXPECT_EQ(ButtonEvent::PRESSED, button.nextEvent());
    EXPECT_TRUE(pin.isInterruptOn);

    pin.high = true;
    rt.advance(1_counts);
    EXPECT_EQ(ButtonEvent::DOWN, button.nextEvent()); // still debouncing
    EXPECT_TRUE(pin.isInterruptOn); // but interrupt got re-enabled
}

}
