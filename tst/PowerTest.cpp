#include <gtest/gtest.h>
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Mocks.hpp"
#include "invoke.hpp"

namespace PowerTest {

using namespace HAL::Atmel;
using namespace Time;
using namespace Mocks;

TEST(PowerTest, sleep_adjusts_timer) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    onSleep_cpu = [&power] {
        // simulate always waking up by watchdog
        invoke<Int_WDT_>(power);
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

TEST(PowerTest, can_sleep_until_deadline) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    auto d = deadline(rt, 1_s);
    onSleep_cpu = [&power] {
        // simulate always waking up by watchdog
        invoke<Int_WDT_>(power);
    };

    power.sleepUntil(d, SleepMode::POWER_DOWN);

    EXPECT_EQ(992, rt.slept); // nearest multiple of 16

    onSleep_cpu = nullptr;
}

TEST(PowerTest, can_sleep_until_periodic) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    auto d = periodic(rt, 1_s);
    onSleep_cpu = [&power] {
        // simulate always waking up by watchdog
        invoke<Int_WDT_>(power);
    };

    power.sleepUntil(d, SleepMode::POWER_DOWN);

    EXPECT_EQ(992, rt.slept); // nearest multiple of 16

    onSleep_cpu = nullptr;
}

TEST(PowerTest, can_sleep_until_closest_deadline) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    auto d1 = deadline(rt, 2_s);
    auto d2 = deadline(rt, 1_s);

    onSleep_cpu = [&power] {
        // simulate always waking up by watchdog
        invoke<Int_WDT_>(power);
    };

    power.sleepUntilAny(SleepMode::POWER_DOWN, d1, d2);

    EXPECT_EQ(992, rt.slept); // nearest multiple of 16

    onSleep_cpu = nullptr;
}

TEST(PowerTest, unscheduled_deadline_allows_unrestricted_sleep) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    auto d1 = deadline(rt, 1_ms);
    d1.cancel();

    power.sleepUntil(d1, SleepMode::POWER_DOWN);

    EXPECT_EQ(4096, rt.slept);
}

TEST(PowerTest, should_not_sleep_on_delays_less_than_16ms) {
    MockRealTimer rt;
    auto power = Power<MockRealTimer>(rt);
    bool haveSlept = false;
    onSleep_cpu = [&haveSlept] { haveSlept = true; };
    power.sleepFor(Milliseconds<>(1), SleepMode::POWER_DOWN);

    EXPECT_FALSE(haveSlept);
}

}
