/*
 * PulseTxTest.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: jan
 */

#include "PulseTx.hpp"
#include <gtest/gtest.h>

namespace PulseTxTest {

struct MockComparator {
    typedef uint8_t value_t;

    bool isInterruptOn = false;
    InterruptChain i;
    value_t target = 0;
    NonPWMOutputMode outputMode = NonPWMOutputMode::disconnected;

    void output(NonPWMOutputMode mode) {
        outputMode = mode;
    }

    InterruptChain &interrupt() {
        return i;
    }

    void setTarget(value_t _target) {
        target = _target;
    }

    void interruptOn() {
        isInterruptOn = true;
    }

    void interruptOff() {
        isInterruptOn = false;
    }

    value_t getValue() {
        return 5;
    }
};

struct MockSoftwarePin {
    bool high = false;
    void setHigh (bool h) {
        high = h;
    }
};

TEST(PulseTxTest, single_pulse_on_software_target_can_be_sent) {
    MockComparator comparator;
    MockSoftwarePin pin;
    SoftwarePulseTx<MockComparator,MockSoftwarePin> tx = { comparator, pin };
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(comparator.isInterruptOn);

    tx.pulse(true, 50);
    EXPECT_TRUE(pin.high);
    EXPECT_TRUE(comparator.isInterruptOn);
    EXPECT_EQ(55, comparator.target);

    comparator.i.invoke();
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(comparator.isInterruptOn);
}

struct MockSource {
    uint8_t index = 0;

    bool hasNextPulse() {
        return index < 2;
    }
    inline uint8_t getNextPulseDuration() {
        index++;
        return (index == 1) ? 42 : (index == 2) ? 10 : 0;
    }

};

TEST(PulseTxTest, multiple_pulses_on_software_target_can_be_sent) {
    MockComparator comparator;
    MockSoftwarePin pin;
    MockSource source;
    SoftwarePulseTx<MockComparator,MockSoftwarePin,MockSource> tx = { comparator, pin, source };
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(comparator.isInterruptOn);

    tx.pulse(true, 50);
    EXPECT_TRUE(pin.high);
    EXPECT_TRUE(comparator.isInterruptOn);
    EXPECT_EQ(55, comparator.target);

    comparator.i.invoke();
    EXPECT_FALSE(pin.high);
    EXPECT_TRUE(comparator.isInterruptOn);
    EXPECT_EQ(97, comparator.target);

    comparator.i.invoke();
    EXPECT_TRUE(pin.high);
    EXPECT_TRUE(comparator.isInterruptOn);
    EXPECT_EQ(107, comparator.target);

    comparator.i.invoke();
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(comparator.isInterruptOn);
}

struct MockHardwarePin {
    typedef MockComparator comparator_t;
    comparator_t comp;

    inline comparator_t &timerComparator() {
        return comp;
    }
    bool high = false;
    void setHigh (bool h) {
        high = h;
    }
    void configureAsOutput() {

    }
};

TEST(PulseTxTest, single_pulse_on_comparators_pwm_pin_can_be_sent) {
    MockHardwarePin pin;
    HardwarePulseTx<MockHardwarePin> tx = { pin };
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(pin.comp.isInterruptOn);
    EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comp.outputMode);

    tx.pulse(true, 50);
    EXPECT_TRUE(pin.high);
    EXPECT_TRUE(pin.comp.isInterruptOn);
    EXPECT_EQ(55, pin.comp.target);
    EXPECT_EQ(NonPWMOutputMode::low_on_match, pin.comp.outputMode);

    pin.comp.i.invoke();
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(pin.comp.isInterruptOn);
    EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comp.outputMode);
}

TEST(PulseTxTest, multiple_pulses_on_comparators_pwm_pin_can_be_sent) {
    MockHardwarePin pin;
    MockSource source;
    HardwarePulseTx<MockHardwarePin,MockSource> tx = { pin, source };
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(pin.comp.isInterruptOn);
    EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comp.outputMode);

    tx.pulse(true, 50);
    EXPECT_TRUE(pin.high);
    EXPECT_TRUE(pin.comp.isInterruptOn);
    EXPECT_EQ(55, pin.comp.target);
    EXPECT_EQ(NonPWMOutputMode::low_on_match, pin.comp.outputMode);

    pin.comp.i.invoke();
    EXPECT_TRUE(pin.comp.isInterruptOn);
    EXPECT_EQ(97, pin.comp.target);
    EXPECT_EQ(NonPWMOutputMode::high_on_match, pin.comp.outputMode);

    pin.comp.i.invoke();
    EXPECT_TRUE(pin.comp.isInterruptOn);
    EXPECT_EQ(107, pin.comp.target);
    EXPECT_EQ(NonPWMOutputMode::low_on_match, pin.comp.outputMode);

    pin.comp.i.invoke();
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(pin.comp.isInterruptOn);
    EXPECT_EQ(NonPWMOutputMode::disconnected, pin.comp.outputMode);
}

}
