#include "PIR/HCSR501.hpp"
#include <gtest/gtest.h>
#include "Mocks.hpp"

namespace HCSR501Test {

using namespace Mocks;
using namespace PIR;

TEST(HCSR501Test, should_ignore_transitions_during_initialization) {
    MockPin data, power;
    MockRealTimer rt;
    auto pir = HCSR501(data, power, rt);

    EXPECT_FALSE(data.isOutput);
    EXPECT_TRUE(power.isOutput);
    EXPECT_TRUE(power.high);
    EXPECT_FALSE(data.isInterruptOn);
    EXPECT_FALSE(pir.isMotionDetected());
}

TEST(HCSR501Test, should_sleep_after_detecting_motion) {
    MockPin data, power;
    MockRealTimer rt;
    auto pir = HCSR501(data, power, rt);

    rt.advance(1_min);
    pir.loop();
    EXPECT_TRUE(data.isInterruptOn);
    EXPECT_FALSE(pir.isMotionDetected());

    data.high = true;
    decltype(pir)::onPinRisingHandler::invoke(&pir);
    EXPECT_TRUE(pir.isMotionDetected());
    EXPECT_FALSE(power.high);
    EXPECT_FALSE(data.isInterruptOn);

    data.high = false;
    rt.advance(15_min);
    pir.loop();
    EXPECT_TRUE(power.high);
    EXPECT_FALSE(data.isInterruptOn);

    rt.advance(1_min);
    pir.loop();
    EXPECT_TRUE(data.isInterruptOn);
}

}
