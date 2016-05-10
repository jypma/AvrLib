#ifndef MOCKS_HPP_
#define MOCKS_HPP_

#include "HAL/Atmel/InterruptVectors.hpp"
#include "Time/Units.hpp"
#include <gtest/gtest.h>

namespace Mocks {

using namespace Time;

struct MockComparator {
    constexpr static uint8_t prescalerPower2 = 3; // prescaler is 2^3 = 8

    typedef uint8_t value_t;
    typedef INTERRUPT_VECTOR(TIMER0_COMPA) INT;

    value_t value = 0;
    value_t target = 0;
    bool isInterruptOn = false;

    void advance() {
        value++;
    }

    template<typename duration_t>
    void advance(duration_t duration) {
        value += value_t(toCountsOn<MockComparator>(duration));
    }

    void interruptOff() {
        isInterruptOn = false;
    }

    void interruptOn() {
        isInterruptOn = true;
    }

    value_t getValue() {
        return value;
    }

    void setTarget(value_t t) {
        target = t;
    }
};

struct MockPin {
    typedef INTERRUPT_VECTOR(INT0) INT;

    bool isOutput = true;
    bool high = false;
    bool isInterruptOn = false;

    void configureAsInputWithPullup() {
        isOutput = false;
    }

    void configureAsInputWithoutPullup() {
        isOutput = false;
    }

    void configureAsOutput() {
        isOutput = true;
    }

    void setLow() {
        EXPECT_TRUE(isOutput);
        high = false;
    }

    void setHigh() {
        EXPECT_TRUE(isOutput);
        high = true;
    }

    bool isHigh() {
        return high;
    }

    void interruptOnChange() {
        isInterruptOn = true;
    }

    void interruptOnRising() {
        isInterruptOn = true;
    }

    void interruptOff() {
        isInterruptOn = false;
    }
};

template <uint8_t p = 10>
struct MockRealTimerPrescaled {
    constexpr static uint8_t prescalerPower2 = p;
    typedef uint8_t value_t;
    constexpr static uint8_t maximum = 255; // 8-bit timer

    uint32_t c = 0;
    int slept = 0;

    void haveSlept(Milliseconds<> millis) {
        slept += millis.getValue();
    }

    template<typename duration_t>
    void advance(duration_t duration) {
        c += uint32_t(toCountsOn<MockRealTimerPrescaled<p>>(duration));
    }

    uint32_t counts() {
        return c;
    }
};

typedef MockRealTimerPrescaled<10> MockRealTimer;

}



#endif /* MOCKS_HPP_ */
