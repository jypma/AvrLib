#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/TWI.hpp"
#include <gtest/gtest.h>
#include <thread>
#include "invoke.hpp"

namespace TWITest {

using namespace HAL::Atmel;
using namespace HAL::Atmel::Registers;

struct MockTWIInfo {
    struct PinSDA {
        static constexpr auto &DDR = DDB0;
        static constexpr auto &PORT = PORTB0;
    };
    struct PinSCL {
        static constexpr auto &DDR = DDB1;
        static constexpr auto &PORT = PORTB1;
    };
};

    constexpr auto TW = ~(TWPS0 | TWPS1 | TWS3 | TWS4 | TWS5 | TWS6 | TWS7);

    constexpr auto TW_START       = TW | TWS3;
    constexpr auto TW_MT_SLA_ACK  = TW | TWS3 | TWS4;
    constexpr auto TW_MT_DATA_ACK = TW | TWS3 | TWS5;
    constexpr auto TW_MR_SLA_ACK  = TW | TWS6;
    constexpr auto TW_MR_DATA_NACK= TW | TWS3 | TWS4 | TWS6;
    constexpr auto TW_MR_DATA_ACK = TW | TWS4 | TWS6;

TEST(TWITest, can_write_a_one_byte_message) {
    TWCR = ~(TWIE | TWEN | TWWC | TWSTO | TWSTA | TWEA | TWINT);

    Impl::TWI<MockTWIInfo,32,32,100000> twi;
    EXPECT_FALSE(twi.isTransceiving());
    twi.write(uint8_t(84), uint8_t(42));
    EXPECT_TRUE(twi.isTransceiving());
    EXPECT_TRUE(TWCR.val() != 0);


    TWSR = TW_START;
    invoke<Int_TWI_>(twi);

    EXPECT_EQ(84 << 1, TWDR.val());
    EXPECT_EQ(TWCR, TWEN | TWIE | TWINT | TWEA);

    TWSR = TW_MT_SLA_ACK;
    invoke<Int_TWI_>(twi);

    EXPECT_EQ(TWEN | TWIE | TWINT | TWEA, TWCR);
    EXPECT_EQ(42, TWDR.val());

    // launch thread that clears TWSTO in TWCR
    volatile bool running = true;
    std::thread t([&](){
		while(running) {
			TWCR &= ~TWSTO;
		}
	});

    TWSR = TW_MT_DATA_ACK;
    invoke<Int_TWI_>(twi);

    running = false;
    t.join();

    EXPECT_FALSE(twi.isTransceiving());
}

TEST(TWITest, can_read_a_one_byte_message_ended_with_NACK) {
    TWCR = ~(TWIE | TWEN | TWWC | TWSTO | TWSTA | TWEA | TWINT);
    sei();

    Impl::TWI<MockTWIInfo,32,32,100000> twi;

    volatile bool running = true;
    std::thread t([&](){
    	std::this_thread::sleep_for(std::chrono::milliseconds(100));

        EXPECT_TRUE(twi.isTransceiving());
        EXPECT_TRUE(TWCR.val() != 0);

        TWSR = TW_START;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ((84 << 1) | 1, TWDR.val());
        EXPECT_EQ(TWEN | TWIE | TWINT | TWEA, TWCR);

        TWSR = TW_MR_SLA_ACK;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ(TWEN | TWIE | TWINT | TWEA, TWCR);

        TWSR = TW_MR_DATA_ACK;
        TWDR.val() = 123;
        invoke<Int_TWI_>(twi);

        EXPECT_EQ(TWEN | TWIE | TWINT, TWCR);

        TWSR = TW_MR_DATA_NACK;
        TWDR.val() = 255;
        invoke<Int_TWI_>(twi);

		while(running) {
			TWCR |= TWSTO;
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

