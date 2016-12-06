#include <gtest/gtest.h>
#include "Time/Units.hpp"

using namespace Time;

namespace UnitsTest {

template <int p>
struct timer_t {
    static constexpr uint8_t maximumPower2 = 8;
    static constexpr uint32_t maximum = 256;
    static constexpr uint8_t prescalerPower2 = p;
};

TEST(UnitsTest, Milliseconds_toTicksOn_should_handle_edge_cases_for_slow_timer) {
    typedef timer_t<10> T; // 8-bit timer, prescaler 1024

    EXPECT_EQ(Ticks(0), Milliseconds(0).toTicks<T>());
    EXPECT_EQ(Ticks(0), Milliseconds(1).toTicks<T>());
    EXPECT_EQ(Ticks(250679024), Milliseconds(0xFFFFFFFF).toTicks<T>());
}

TEST(UnitsTest, Milliseconds_toTicksOn_should_handle_overflows_for_fast_timer) {
    typedef timer_t<0> T; // 8-bit timer, prescaler 1 (2^0)

    EXPECT_EQ(Ticks(0), Milliseconds(0).toTicks<T>());
    EXPECT_EQ(Ticks(62), Milliseconds(1).toTicks<T>());
    EXPECT_EQ(Ticks(0xFFFFFFFF), Milliseconds(0xFFFFFFFF).toTicks<T>());
}

TEST(UnitsTest, Counts_toMillisOn_should_handle_edge_cases_for_slow_timer) {
    typedef timer_t<10> T; // 8-bit timer, prescaler 1024

    EXPECT_EQ(Milliseconds(0), Counts(0).toMillis<T>());
    EXPECT_EQ(Milliseconds(0), Counts(1).toMillis<T>());
    EXPECT_EQ(Milliseconds(960), Counts(15000).toMillis<T>());
    EXPECT_EQ(Milliseconds(274877920), Counts(0xFFFFFFFF).toMillis<T>());
}

TEST(UnitsTest, Counts_toMillisOn_should_handle_overflows_for_very_slow_timer) {
    typedef timer_t<16> T; // 8-bit timer, prescaler 65K

    EXPECT_EQ(Milliseconds(0), Counts(0).toMillis<T>());
    EXPECT_EQ(Milliseconds(4), Counts(1).toMillis<T>());
    EXPECT_EQ(Milliseconds(0xFFFFFFFF), Counts(0xFFFFFFFF).toMillis<T>());
}

}
