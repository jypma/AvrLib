#include <gtest/gtest.h>
#include "HAL/Atmel/Power.hpp"

namespace PowerTest {

using namespace HAL::Atmel;
using namespace Time;

struct MockRealTimer {
    int slept = 0;

    void haveSlept(Milliseconds<> millis) {
        slept += millis.getValue();
    }
};

TEST(PowerTest, sleep_adjusts_timer) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    onSleep_cpu = [&power] {
        // simulate always waking up by watchdog
        decltype(power)::onWatchdogHandler::invoke(power);
    };
    power.sleepFor(Milliseconds<>(1000), SleepMode::POWER_DOWN);
    EXPECT_EQ(992, rt.slept); // nearest multiple of 16

    onSleep_cpu = nullptr;
}

TEST(PowerTest, sleep_returns_if_woken_up_by_non_watchdog) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    onSleep_cpu = nullptr;  // don't invoke watchdog when waking up.
    power.sleepFor(Milliseconds<>(1000), SleepMode::POWER_DOWN);

    // The code requested to sleep for 512ms, but since we don't know when during that time we
    // woke up, we return half that time.
    EXPECT_EQ(256, rt.slept);
}

TEST(PowerTest, sleep_with_IDLE_does_not_adjust_timer) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    power.sleepFor(Milliseconds<>(1000), SleepMode::IDLE);
    EXPECT_EQ(0, rt.slept);
}

}
