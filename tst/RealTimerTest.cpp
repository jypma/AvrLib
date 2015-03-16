#include "RealTimer.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

struct MockTimer {
    InterruptChain interrupt;
    uint8_t value = 0;
    static constexpr uint8_t maximumPower2 = 8;
    static constexpr uint8_t prescalerPower2 = 8;

    uint8_t getValue() {
        return value;
    }

    InterruptChain &interruptOnOverflow() {
        return interrupt;
    }
    void interruptOnOverflowOn() {

    }
};


MockTimer t1;
TEST(RealTimerTest, timer_callback_increments_time) {
    RealTimer<MockTimer, &t1> rt;

    EXPECT_EQ(0, rt.counts());
    EXPECT_EQ(0, rt.millis());
    EXPECT_EQ(0, rt.micros());

    t1.interrupt.invoke();

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
    RealTimer<MockTimer, &t2, 0, &wait2> rt;

    std::thread background([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        t2.interrupt.invoke();
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
    RealTimer<MockTimer, &t3, 0xFFFFFFFF, &wait3> rt;

    std::thread background([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        t3.interrupt.invoke();
    });

    waited3 = false;
    rt.delayTicks(1);
    background.join();

    EXPECT_TRUE(waited3);
}
