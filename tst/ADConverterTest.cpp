#include "HAL/Atmel/ADConverter.hpp"
#include <gtest/gtest.h>

namespace ADCTest {

using namespace HAL::Atmel;

struct MockPin {
    struct info_t {
        static constexpr auto adc_mux = ~(MUX0 | MUX1 | MUX2 | MUX3);
    };
};

TEST(ADCTest, ADC16_can_template) {
    MockPin pin;
    ADConverter<uint16_t> adc;
    adc.measure(pin);
}

TEST(ADCTest, ADC8_can_template) {
    MockPin pin;
    ADConverter<uint8_t> adc;
    adc.measure(pin);
}

}
