#ifndef LDRTEST_CPP_
#define LDRTEST_CPP_

#include <gtest/gtest.h>
#include <Passive/LogResistor.hpp>
#include "Mocks.hpp"

using namespace Passive;
using namespace Mocks;

namespace LogResistorTest {

TEST(LogResistorTest, can_measure_a_value_twice) {
    MockRealTimerPrescaled<3> rt;
    MockPin pin_in, pin_out;
    pin_in.high = false;

    auto ldr = LogResistor<2600, 500, 10000000>(rt, pin_out, pin_in);

    EXPECT_TRUE(pin_out.isOutput);
    EXPECT_FALSE(pin_out.high);
    EXPECT_FALSE(pin_in.isOutput);
    EXPECT_FALSE(pin_in.isInterruptOn);
    EXPECT_FALSE(ldr.isMeasuring());

    ldr.measure();
    EXPECT_TRUE(pin_out.high);
    EXPECT_TRUE(pin_in.isInterruptOn);
    EXPECT_TRUE(ldr.isMeasuring());

    rt.advance(500_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_FALSE(pin_in.isInterruptOn);
    EXPECT_FALSE(ldr.isMeasuring());
    EXPECT_EQ(50, ldr.getValue());

    ldr.measure();
    EXPECT_TRUE(pin_in.isInterruptOn);
    EXPECT_TRUE(ldr.isMeasuring());

    rt.advance(5000_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_FALSE(pin_in.isInterruptOn);
    EXPECT_FALSE(ldr.isMeasuring());
    EXPECT_EQ(83, ldr.getValue());
}

TEST(LogResistorTest, clips_lower_and_upper_measurement_bounds_to_0_and_255) {
    MockRealTimerPrescaled<3> rt;
    MockPin pin_in, pin_out;

    auto ldr = LogResistor<2600, 500, 10000000>(rt, pin_out, pin_in);

    ldr.measure();
    rt.advance(1_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(0, ldr.getValue());

    ldr.measure();
    rt.advance(1000000_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(255, ldr.getValue());
}

TEST(LogResistorTest, response_is_somewhat_logarithmic_over_time) {
    MockRealTimerPrescaled<3> rt;
    MockPin pin_in, pin_out;

    auto ldr = LogResistor<2600, 500, 10000000>(rt, pin_out, pin_in);

    ldr.measure();
    rt.advance(20_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(18, ldr.getValue());

    ldr.measure();
    rt.advance(200_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(46, ldr.getValue());

    ldr.measure();
    rt.advance(2000_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(70, ldr.getValue());

    ldr.measure();
    rt.advance(5000_counts);
    decltype(ldr)::onPinChangeHandler::invoke(&ldr);
    EXPECT_EQ(83, ldr.getValue());
}

TEST(LogResistorTest, times_out_if_pin_doesnt_change_in_time) {
    MockRealTimerPrescaled<3> rt;
    MockPin pin_in, pin_out;

    auto ldr = LogResistor<2600, 500, 10000000>(rt, pin_out, pin_in);

    ldr.measure();
    rt.advance(1000000_counts);

    EXPECT_FALSE(ldr.isMeasuring());
    EXPECT_EQ(107, ldr.getValue()); // the maximum value in the current non-scaling code
    EXPECT_FALSE(pin_out.high);
    EXPECT_FALSE(pin_out.isInterruptOn);
}

}



#endif /* LDRTEST_CPP_ */
