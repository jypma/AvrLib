#ifndef MOCKS_HPP_
#define MOCKS_HPP_

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "HAL/Atmel/Timer.hpp"
#include "Time/Units.hpp"
#include "Time/TimerValue.hpp"
#include <gtest/gtest.h>
#include "Fifo.hpp"
#include <limits.h>
#include "invoke.hpp"

namespace Mocks {

using namespace Time;
using namespace HAL::Atmel;

template <typename _value_t = uint8_t, uint8_t _prescalerPower2 = 3>
struct MockComparator {
    constexpr static uint8_t prescalerPower2 = _prescalerPower2;

    typedef _value_t value_t;
    typedef TimerValue<MockComparator<_value_t, _prescalerPower2>> timervalue_t;
    typedef Int_TIMER0_COMPA_ INT;

    value_t value = 0;
    value_t target = 0;
    bool isInterruptOn = false;
    NonPWMOutputMode mode = NonPWMOutputMode::disconnected;

    void advance() {
        value++;
    }

    void advanceToTarget() {
    	value = target;
    }

    template <typename target_t>
    void advanceToTargetAndInvoke(target_t &t) {
    	advanceToTarget();
    	invoke<INT>(t);
    }

    template<typename duration_t>
    void advance(duration_t duration) {
        value += value_t(toCountsOn<MockComparator>(duration).getValue());
    }

    void interruptOff() {
        isInterruptOn = false;
    }

    void interruptOn() {
        isInterruptOn = true;
    }

    timervalue_t getValue() const {
        return value;
    }

    void setTarget(timervalue_t t) {
        target = t;
    }

    timervalue_t getTarget() {
    	return target;
    }

    void setOutput(NonPWMOutputMode m) {
    	mode = m;
    }

    void applyOutput() {
    	EXPECT_TRUE(mode != NonPWMOutputMode::disconnected);
    }

    NonPWMOutputMode getOutput() {
    	return mode;
    }
};

struct MockPin {
    typedef Int_INT0_ INT;

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

    void setHigh(bool b) {
        EXPECT_TRUE(isOutput);
        high = b;
    }

    bool isOutputHigh() {
    	EXPECT_TRUE(isOutput);
    	return high;
    }

    void configureAsOutputLow() {
        configureAsOutput();
        setLow();
    }

    void configureAsOutputHigh() {
        configureAsOutput();
        setHigh();
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

template <typename _value_t = uint8_t, uint8_t _prescalerPower2 = 3>
struct MockPinOnComparator: public MockPin {
	typedef MockComparator<_value_t, _prescalerPower2> comparator_t;
	comparator_t comparator;

	comparator_t &timerComparator() { return comparator; }
};

template <uint8_t p = 10>
struct MockRealTimerPrescaled {
    constexpr static uint8_t prescalerPower2 = p;
    typedef uint8_t value_t;
    constexpr static uint8_t maximum = 255; // 8-bit timer

    uint32_t c = 0;
    int slept = 0;

    void haveSlept(Milliseconds millis) {
        slept += millis.getValue();
    }

    template<typename duration_t>
    void advance(duration_t duration) {
        c += uint32_t(toCountsOn<MockRealTimerPrescaled<p>>(duration).getValue());
    }

    uint32_t counts() {
        return c;
    }

    uint32_t ticks() {
    	return c / 256;
    }
};

typedef MockRealTimerPrescaled<10> MockRealTimer;

}

struct MockTWI {
    Fifo<32> out;
    Fifo<32> in;
    uint8_t writeAddress;
    uint8_t readAddress;

    template <typename... types> void write(uint8_t address, types... args) {
        writeAddress = address;
        out.write(args...);
    }
    template <typename... types> Streams::ReadResult read(uint8_t address, types... args) {
        readAddress = address;
        return in.read(args...);
    }
};

#endif /* MOCKS_HPP_ */
