#include "HAL/Atmel/TWI.hpp"
#include <gtest/gtest.h>
#include "invoke.hpp"

namespace TWITest {

using namespace HAL::Atmel;
using namespace HAL::Atmel::Impl;

struct MockTWIInfo {
    struct PinSDA {
        static constexpr uint8_t * const ddr = &DDRB;
        static constexpr uint8_t * const port = &PORTB;
        static constexpr uint8_t bitmask = 1;
    };
    struct PinSCL {
        static constexpr uint8_t * const ddr = &DDRB;
        static constexpr uint8_t * const port = &PORTB;
        static constexpr uint8_t bitmask = 2;
    };
};

TEST(TWITest, should_transmit_a_single_byte) {
    TWCR = 0;

    TWI<MockTWIInfo,32,32,100000> twi;
    twi.write(uint8_t(84), uint8_t(42));
    EXPECT_TRUE(TWCR != 0);

    TWSR = TW_START;
    invoke<Int_TWI_>(twi);
}

}

