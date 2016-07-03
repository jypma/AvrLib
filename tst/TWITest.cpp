#include "HAL/Atmel/TWI.hpp"
#include <gtest/gtest.h>
#include <thread>
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

TEST(TWITest, can_write_a_one_byte_message) {
    TWCR = 0;

    TWI<MockTWIInfo,32,32,100000> twi;
    EXPECT_FALSE(twi.isTransceiving());
    twi.write(uint8_t(84), uint8_t(42));
    EXPECT_TRUE(twi.isTransceiving());
    EXPECT_TRUE(TWCR != 0);

    TWSR = TW_START;
    invoke<Int_TWI_>(twi);

    EXPECT_EQ(84 << 1, TWDR);
    EXPECT_EQ(_BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA), TWCR);

    TWSR = TW_MT_SLA_ACK;
    invoke<Int_TWI_>(twi);

    EXPECT_EQ(_BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA), TWCR);
    EXPECT_EQ(42, TWDR);

    // launch thread that clears TWSTO in TWCR
    volatile bool running = true;
    std::thread t([&](){
		while(running) {
			TWCR |= ~_BV(TWSTO);
		}
	});

    TWSR = TW_MT_DATA_ACK;
    invoke<Int_TWI_>(twi);

    running = false;
    t.join();

    EXPECT_FALSE(twi.isTransceiving());
}

TEST(TWITest, can_read_a_one_byte_message_ended_with_NACK) {
    TWCR = 0;
    sei();

    TWI<MockTWIInfo,32,32,100000> twi;

    volatile bool running = true;
    std::thread t([&](){
    	std::this_thread::sleep_for(std::chrono::milliseconds(100));

        EXPECT_TRUE(twi.isTransceiving());
        EXPECT_TRUE(TWCR != 0);

        TWSR = TW_START;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ((84 << 1) | 1, TWDR);
        EXPECT_EQ(_BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA), TWCR);

        TWSR = TW_MR_SLA_ACK;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ(_BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA), TWCR);

        TWSR = TW_MR_DATA_ACK;
        TWDR = 123;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ(_BV(TWEN) | _BV(TWIE) | _BV(TWINT), TWCR);

        TWSR = TW_MR_DATA_NACK;
        TWDR = 255;
        invoke<Int_TWI_>(twi);

		while(running) {
			TWCR |= ~_BV(TWSTO);
		}
	});

    uint8_t b;
    twi.read(uint8_t(84), &b);

    running = false;
    t.join();

    EXPECT_FALSE(twi.isTransceiving());
    EXPECT_EQ(123, b);
}

}

