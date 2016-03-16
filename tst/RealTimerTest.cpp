#include <avr/common.h>
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

namespace RealTimerTest {

using namespace Time;

struct MockTimer {
    typedef HAL::Atmel::InterruptVectors::VectorTIMER0_OVF_ INT;

    typedef uint8_t value_t;
    typedef uint8_t prescaler_t;
    uint8_t value = 0;
    static constexpr uint8_t maximumPower2 = 8;
    static constexpr uint8_t prescaler = 0;
    static constexpr uint8_t prescalerPower2 = 8;

    uint8_t getValue() {
        return value;
    }

    void interruptOnOverflowOn() {

    }
};


MockTimer t1;
TEST(RealTimerTest, timer_callback_increments_time) {
    auto rt = realTimer(t1);

    EXPECT_EQ(0, rt.counts());
    EXPECT_EQ(0, rt.millis());
    EXPECT_EQ(0, rt.micros());

    decltype(rt)::onTimerOverflowHandler::invoke(&rt);

    EXPECT_EQ(256, rt.counts());
    EXPECT_EQ(4, rt.millis());
    EXPECT_EQ(4096, rt.micros());
}

bool waited2;
void wait2() {
    waited2 = true;
}
MockTimer t2;
TEST(RealTimerTest, delayTicks_returns_when_interrupt_is_called) {
    RealTimer<MockTimer, 0, &wait2> rt(t2);

    std::thread background([&rt]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        decltype(rt)::onTimerOverflowHandler::invoke(&rt);
    });

    waited2 = false;
    rt.delayTicks(1);
    background.join();

    EXPECT_TRUE(waited2);
}

bool waited3;
void wait3() {
    waited3 = true;
}
MockTimer t3;
TEST(RealTimerTest, delayTicks_returns_when_interrupt_is_called_during_wraparound) {
    RealTimer<MockTimer, 0xFFFFFFFF, &wait3> rt(t3);

    std::thread background([&rt]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        decltype(rt)::onTimerOverflowHandler::invoke(&rt);
    });

    waited3 = false;
    rt.delayTicks(1);
    background.join();

    EXPECT_TRUE(waited3);
}

struct MockRealTimer {
    typedef uint16_t value_t;
    static constexpr uint64_t maximum = 10000000000;

    uint32_t count = 0;
    uint32_t tick = 0;
    uint32_t counts() const {
        return count;
    }

    uint32_t ticks() const {
        return tick;
    }

};

TEST(RealTimerTest, periodic_catches_up_after_long_invocation) {
    auto rt = MockRealTimer();
    auto p = periodic(rt, 200_counts);

    EXPECT_FALSE(p.isNow());
    EXPECT_FALSE(p.isNow());

    rt.count = 201;

    EXPECT_TRUE(p.isNow());
    EXPECT_FALSE(p.isNow());
    EXPECT_FALSE(p.isNow());

    rt.count = 601;

    EXPECT_TRUE(p.isNow());
    EXPECT_TRUE(p.isNow());
    EXPECT_FALSE(p.isNow());
    EXPECT_FALSE(p.isNow());
}

TEST(RealTimerTest, periodic_returns_time_left) {
    auto rt = MockRealTimer();
    auto p = periodic(rt, 200_counts);
    EXPECT_EQ(Counts<>(200), p.timeLeft());
}

TEST(RealTimerTest, unelapsed_deadline_has_no_time_left) {
    auto rt = MockRealTimer();
    auto p = deadline(rt, 200_counts);
    EXPECT_EQ(Counts<>(200), p.timeLeft());
    rt.count = 201;
    EXPECT_EQ(Counts<>(0), p.timeLeft());
    rt.count = 401;
    EXPECT_EQ(Counts<>(0), p.timeLeft());
}

TEST(RealTimerTest, elapsed_deadline_has_infinite_time_left) {
    auto rt = MockRealTimer();
    auto p = deadline(rt, 200_counts);
    EXPECT_EQ(Counts<>(200), p.timeLeft());
    rt.count = 201;
    EXPECT_TRUE(p.isNow());
    EXPECT_EQ(Counts<>(0xFFFFFFFF), p.timeLeft());
    rt.count = 401;
    EXPECT_EQ(Counts<>(0xFFFFFFFF), p.timeLeft());
}

TEST(RealTimerTest, deadline_only_fires_once) {
    auto rt = MockRealTimer();
    auto d = deadline(rt, 200_counts);

    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    rt.count = 201;

    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    rt.count = 601;

    EXPECT_FALSE(d.isNow());
}

TEST(RealTimerTest, deadline_can_be_reset) {
    auto rt = MockRealTimer();
    auto d = deadline(rt, 200_counts);
    rt.count = 100;
    d.reset();

    rt.count = 201;
    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    rt.count = 301;
    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());

    d.reset();
    EXPECT_FALSE(d.isNow());

    rt.count = 502;
    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());
}

TEST(RealTimerTest, deadline_picks_ticks_instead_of_counts_when_interval_doesnt_fit_32bit) {
    auto rt = MockRealTimer();
    auto d = deadline(rt, 40000000000_counts);

    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    rt.tick = 601;

    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    rt.tick = 1201;

    EXPECT_FALSE(d.isNow());
}

TEST(RealTimerTest, deadline_copes_with_timer_wraparound) {
    auto rt = MockRealTimer();

    rt.count = 0xFFFFFF00;
    auto d = deadline(rt, 400_counts);
    EXPECT_FALSE(d.isNow());
    rt.count = 400;
    EXPECT_TRUE(d.isNow());
}

TEST(RealTimerTest, variable_deadline_can_be_set_to_varying_timeouts) {
    auto rt = MockRealTimer();
    auto d = deadline(rt);

    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    d.reset(200_counts);
    rt.count = 201;

    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());

    d.reset(400_counts);
    rt.count = 402;

    EXPECT_FALSE(d.isNow());

    rt.count = 602;
    EXPECT_TRUE(d.isNow());
    EXPECT_FALSE(d.isNow());
}

TEST(RealTimerTest, variable_deadline_does_not_fire_after_cancel) {
    auto rt = MockRealTimer();
    auto d = deadline(rt);

    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

    d.reset(200_counts);
    rt.count = 201;
    d.cancel();

    EXPECT_FALSE(d.isNow());
    EXPECT_FALSE(d.isNow());

}

TEST(RealTimerTest, variable_deadline_copes_with_timer_wraparound) {
    auto rt = MockRealTimer();
    auto d = deadline(rt);

    rt.count = 0xFFFFFF00;
    d.reset(400_counts);
    rt.count = 400;
    EXPECT_TRUE(d.isNow());
}

TEST(RealTimerTest, animator_runs_forward) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    EXPECT_EQ(AnimatorEvent(false, false, 0), a.nextEvent());

    a.from(256).to(0xFFFF, 100_counts);

    EXPECT_EQ(AnimatorEvent(true, true, 256), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 256), a.nextEvent());
    rt.count = 1;
    EXPECT_EQ(AnimatorEvent(true, true, 908), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 908), a.nextEvent());
    rt.count = 2;
    EXPECT_EQ(AnimatorEvent(true, true, 1561), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 1561), a.nextEvent());
    rt.count = 98;
    EXPECT_EQ(AnimatorEvent(true, true, 64229), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 64229), a.nextEvent());
    rt.count = 99;
    EXPECT_EQ(AnimatorEvent(true, true, 64882), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 64882), a.nextEvent());
    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(false, true, 0xFFFF), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(false, false, 0xFFFF), a.nextEvent());
    rt.count = 101;
    EXPECT_EQ(AnimatorEvent(false, false, 0xFFFF), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_forward_easing_in) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    a.from(0).to(0xFFFF, 15000_counts, AnimatorInterpolation::EASE_IN);

    EXPECT_EQ(AnimatorEvent(true, true, 0), a.nextEvent());
    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(true, true, 2), a.nextEvent());
    rt.count = 200;
    EXPECT_EQ(AnimatorEvent(true, true, 11), a.nextEvent());
    rt.count = 9800;
    EXPECT_EQ(AnimatorEvent(true, true, 27973), a.nextEvent());
    rt.count = 14000;
    EXPECT_EQ(AnimatorEvent(true, true, 57088), a.nextEvent());
    rt.count = 15000;
    EXPECT_EQ(AnimatorEvent(false, true, 0xFFFF), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_backward_easing_up) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    a.from(2000).to(0, 15000_counts, AnimatorInterpolation::EASE_UP);

    EXPECT_EQ(AnimatorEvent(true, true, 2000), a.nextEvent());
    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(true, true, 1974), a.nextEvent());
    rt.count = 200;
    EXPECT_EQ(AnimatorEvent(true, true, 1948), a.nextEvent());
    rt.count = 9800;
    EXPECT_EQ(AnimatorEvent(true, true, 241), a.nextEvent());
    rt.count = 14000;
    EXPECT_EQ(AnimatorEvent(true, true, 9), a.nextEvent());
    rt.count = 15000;
    EXPECT_EQ(AnimatorEvent(false, true, 0), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_forward_easing_in_2) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    a.from(0).to(2000, 15000_counts, AnimatorInterpolation::EASE_IN);

    EXPECT_EQ(AnimatorEvent(true, true, 0), a.nextEvent());
    rt.count = 1000;
    EXPECT_EQ(AnimatorEvent(true, true, 8), a.nextEvent());
    rt.count = 2000;
    EXPECT_EQ(AnimatorEvent(true, true, 35), a.nextEvent());
    rt.count = 9800;
    EXPECT_EQ(AnimatorEvent(true, true, 853), a.nextEvent());
    rt.count = 14000;
    EXPECT_EQ(AnimatorEvent(true, true, 1742), a.nextEvent());
    rt.count = 15000;
    EXPECT_EQ(AnimatorEvent(false, true, 2000), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_forward_easing_out) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    a.from(256).to(0xFFFF, 100_counts, AnimatorInterpolation::EASE_OUT);

    EXPECT_EQ(AnimatorEvent(true, true, 256), a.nextEvent());
    rt.count = 1;
    EXPECT_EQ(AnimatorEvent(true, true, 1555), a.nextEvent());
    rt.count = 2;
    EXPECT_EQ(AnimatorEvent(true, true, 2841), a.nextEvent());
    rt.count = 98;
    EXPECT_EQ(AnimatorEvent(true, true, 65508), a.nextEvent());
    rt.count = 99;
    EXPECT_EQ(AnimatorEvent(true, true, 65528), a.nextEvent());
    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(false, true, 0xFFFF), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_backward) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    a.from(0xFFFF).to(256, 100_counts);

    EXPECT_EQ(AnimatorEvent(true, true, 0xFFFF), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 0xFFFF), a.nextEvent());
    rt.count = 1;
    EXPECT_EQ(AnimatorEvent(true, true, 64883), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 64883), a.nextEvent());
    rt.count = 99;
    EXPECT_EQ(AnimatorEvent(true, true, 909), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 909), a.nextEvent());
    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(false, true, 256), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(false, false, 256), a.nextEvent());
    rt.count = 101;
    EXPECT_EQ(AnimatorEvent(false, false, 256), a.nextEvent());
}

TEST(RealTimerTest, animator_runs_forward_and_backward) {
    auto rt = MockRealTimer();
    auto a = animator(rt);

    EXPECT_EQ(AnimatorEvent(false, false, 0), a.nextEvent());

    a.from(256);
    EXPECT_EQ(AnimatorEvent(false, true, 256), a.nextEvent());

    a.to(512, 100_counts);
    EXPECT_EQ(AnimatorEvent(true, false, 256), a.nextEvent());

    rt.count = 1;
    EXPECT_EQ(AnimatorEvent(true, true, 258), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 258), a.nextEvent());

    rt.count = 100;
    EXPECT_EQ(AnimatorEvent(false, true, 512), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(false, false, 512), a.nextEvent());

    a.to(256, 100_counts);
    EXPECT_EQ(AnimatorEvent(true, false, 512), a.nextEvent());

    rt.count = 101;
    EXPECT_EQ(AnimatorEvent(true, true, 510), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(true, false, 510), a.nextEvent());

    rt.count = 200;
    EXPECT_EQ(AnimatorEvent(false, true, 256), a.nextEvent());
    EXPECT_EQ(AnimatorEvent(false, false, 256), a.nextEvent());

    rt.count = 201;
    EXPECT_EQ(AnimatorEvent(false, false, 256), a.nextEvent());
}
}
