#include <gtest/gtest.h>
#include "Serial/FrequencyCounter.hpp"
#include "invoke.hpp"
#include "Mocks.hpp"

namespace FrequencyCounterTest {

using namespace Serial;
using namespace Mocks;

MockPin pin;
MockRealTimer rt;

TEST(FrequencyCounter, should_have_no_frequency_on_creation) {
    FrequencyCounter<MockPin, MockRealTimer> c = { pin, rt };
    EXPECT_EQ(none(), c.getFrequency());
}

TEST(FrequencyCounter, should_count_if_all_values_equal) {
    FrequencyCounter<MockPin, MockRealTimer> c = { pin, rt };
    invoke<MockPin::INT>(c);
    rt.advance(20_ms);
    invoke<MockPin::INT>(c);
    rt.advance(20_ms);
    invoke<MockPin::INT>(c);
    rt.advance(20_ms);
    invoke<MockPin::INT>(c);
    EXPECT_TRUE(c.getFrequency().isDefined());
    EXPECT_EQ(some(uint16_t(50)), c.getFrequency());
}

TEST(FrequencyCounter, should_count_if_all_values_similar) {
    FrequencyCounter<MockPin, MockRealTimer> c = { pin, rt };
    invoke<MockPin::INT>(c);
    rt.advance(19_ms);
    invoke<MockPin::INT>(c);
    rt.advance(21_ms);
    invoke<MockPin::INT>(c);
    rt.advance(20_ms);
    invoke<MockPin::INT>(c);
    EXPECT_TRUE(c.getFrequency().isDefined());
    EXPECT_EQ(some(uint16_t(50)), c.getFrequency());
}

TEST(FrequencyCounter, should_have_no_frequency_for_rubbish_values) {
    FrequencyCounter<MockPin, MockRealTimer> c = { pin, rt };
    invoke<MockPin::INT>(c);
    rt.advance(19_ms);
    invoke<MockPin::INT>(c);
    rt.advance(1_ms);
    invoke<MockPin::INT>(c);
    rt.advance(20_ms);
    invoke<MockPin::INT>(c);
    EXPECT_EQ(none(), c.getFrequency());
}

}
