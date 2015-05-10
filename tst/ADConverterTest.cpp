#include "ADConverter.hpp"
#include <gtest/gtest.h>

namespace ADCTest {

struct MockPin {
    struct info_t {
        static constexpr uint8_t adc_mux = 0;
    };
};

TEST(ADCTest, ADC_can_template) {
    MockPin pin;
    ADConverter adc;
    adc.start(pin);
}

}
