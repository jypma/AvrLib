#include <gtest/gtest.h>
#include "Time/PWM.hpp"
#include "Mocks.hpp"

namespace PWMTest {

using namespace Mocks;
using namespace Time;

struct MockTimer {
	typedef uint8_t value_t;
	value_t value = 0;
	value_t getValue() { return value; }
};

TEST(PWMTest, should_honor_pulse_width_after_wraparound) {
	MockTimer timer;
	auto pwm = PWM(timer);
	pwm.setPulseWidth(100);
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 99;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 100;
	EXPECT_FALSE(pwm.isHigh());
	timer.value = 101;
	EXPECT_FALSE(pwm.isHigh());
	timer.value = 0;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 100;
	EXPECT_FALSE(pwm.isHigh());
}

TEST(PWMTest, should_shorten_next_pulse_on_late_invocation_and_then_reset) {
	MockTimer timer;
	auto pwm = PWM(timer);
	pwm.setPulseWidth(100);
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 105;
	EXPECT_FALSE(pwm.isHigh());
	timer.value = 0;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 94;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 95;
	EXPECT_FALSE(pwm.isHigh());
	timer.value = 96;
	EXPECT_FALSE(pwm.isHigh());
	timer.value = 0;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 99;
	EXPECT_TRUE(pwm.isHigh());
	timer.value = 100;
	EXPECT_FALSE(pwm.isHigh());
}

}
