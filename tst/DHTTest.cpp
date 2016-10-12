#include <gtest/gtest.h>
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "DHT/DHT11.hpp"
#include "DHT/DHT22.hpp"
#include "invoke.hpp"

namespace DHTTest {

using namespace DHT;
using namespace HAL::Atmel;

struct MockComparator {
    constexpr static uint8_t prescalerPower2 = 3; // prescaler is 2^3 = 8

    typedef uint8_t value_t;
    typedef Int_TIMER0_COMPA_ INT;

    value_t value = 0;
    value_t target = 0;
    bool isInterruptOn = false;

    void advance() {
        value++;
    }

    template<typename duration_t>
    void advance(duration_t duration) {
        value += value_t(toCountsOn<MockComparator>(duration));
    }

    void interruptOff() {
        isInterruptOn = false;
    }

    void interruptOn() {
        isInterruptOn = true;
    }

    value_t getValue() {
        return value;
    }

    void setTarget(value_t t) {
        target = t;
    }
};

struct MockPin {
    typedef Int_INT0_ INT;

    bool isOutput = true;
    bool high = false;
    bool isInterruptOn = false;

    void configureAsInputWithPullup() {
        isOutput = false;
    }

    void configureAsOutput() {
        isOutput = true;
    }

    void setLow() {
        EXPECT_TRUE(isOutput);
        high = false;
    }

    void setHigh() {
        EXPECT_TRUE(isOutput);
        high = true;
    }

    bool isHigh() {
        return high;
    }

    void interruptOnChange() {
        isInterruptOn = true;
    }

    void interruptOff() {
        isInterruptOn = false;
    }
};

struct MockRealTimer {
    constexpr static uint8_t prescalerPower2 = 10; // prescaler is 2^10 = 1024

    uint32_t c = 0;

    template<typename duration_t>
    void advance(duration_t duration) {
        c += uint32_t(toCountsOn<MockRealTimer>(duration));
    }

    uint32_t counts() {
        return c;
    }
};

TEST(DHTTest, powers_on_before_measuring) {
    MockComparator comparator;
    MockPin pin, power;
    MockRealTimer rt;

    auto dht = DHT11(pin, power, comparator, rt);
    dht.powerOff();
    EXPECT_FALSE(pin.isOutput);
    EXPECT_TRUE(power.isOutput);
    EXPECT_FALSE(power.high);

    dht.measure();
    EXPECT_TRUE(power.high);
    EXPECT_EQ(DHTState::BOOTING, dht.getState());

    rt.advance(1_s);
    dht.loop();
    EXPECT_EQ(DHTState::IDLE, dht.getState());
}

TEST(DHTTest, dht11_reads_5_bytes_and_updates_temperature_and_humidity) {
    MockComparator comparator;
    MockPin pin, power;
    MockRealTimer rt;

    auto dht = DHT11(pin, power, comparator, rt);
    EXPECT_FALSE(pin.isOutput);
    EXPECT_TRUE(power.isOutput);
    EXPECT_TRUE(power.high);

    // after 1 second, pin should be pulled low for 18ms
    rt.advance(1_s);
    dht.loop();
    dht.measure();
    EXPECT_EQ(DHTState::SIGNALING, dht.getState());
    EXPECT_TRUE(pin.isOutput);
    EXPECT_FALSE(pin.high);

    // after 18ms, switch to input and wait for low sync pulse
    rt.advance(18_ms);
    dht.loop();
    EXPECT_FALSE(pin.isOutput);
    EXPECT_EQ(DHTState::SYNC_LOW, dht.getState());

    // low sync pulse should take 60..120us
    pin.high = false;
    comparator.advance();
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    std::cout << "----" <<std::endl;
    //dht.onPinChange();
    invoke<MockPin::INT>(dht);
    pin.high = true;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    dht.loop();
    dht.loop();
    EXPECT_EQ(DHTState::SYNC_HIGH, dht.getState());

    // high sync pulse should take 60..120us
    pin.high = false;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    EXPECT_EQ(DHTState::RECEIVING_LOW, dht.getState());

    auto sendTestBit = [&] (bool isOne) {
        pin.high = true;
        comparator.advance(50_us);
        invoke<MockPin::INT>(dht);
        dht.loop();

        pin.high = false;
        if (isOne) {
            comparator.advance(70_us);
        } else {
            comparator.advance(30_us);
        }
        invoke<MockPin::INT>(dht);
        dht.loop();
    };

    auto sendTestByte = [&] (uint8_t data) {
        for (int8_t bit = 7; bit >= 0; bit--) {
            sendTestBit((data & (1 << bit)) != 0);
        }
    };

    sendTestByte(32);
    sendTestByte(0);
    sendTestByte(27);
    sendTestByte(0);
    sendTestByte(59);

    EXPECT_EQ(some(320), dht.getHumidity());
    EXPECT_EQ(some(270), dht.getTemperature());
    EXPECT_EQ(DHTState::IDLE, dht.getState());
    EXPECT_FALSE(pin.isInterruptOn);
    EXPECT_FALSE(comparator.isInterruptOn);

    // Let's take a second measurement
    dht.measure();
    EXPECT_EQ(DHTState::SIGNALING, dht.getState());
    EXPECT_TRUE(pin.isOutput);
    EXPECT_FALSE(pin.high);

    // after 18ms, switch to input and wait for low sync pulse
    rt.advance(18_ms);
    dht.loop();
    EXPECT_FALSE(pin.isOutput);
    EXPECT_EQ(DHTState::SYNC_LOW, dht.getState());

    // low sync pulse should take 60..120us
    pin.high = false;
    comparator.advance();
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    pin.high = true;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    dht.loop();
    dht.loop();
    EXPECT_EQ(DHTState::SYNC_HIGH, dht.getState());

    // high sync pulse should take 60..120us
    pin.high = false;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    EXPECT_EQ(DHTState::RECEIVING_LOW, dht.getState());

    sendTestByte(2);
    sendTestByte(0);
    sendTestByte(42);
    sendTestByte(0);
    sendTestByte(59);

    EXPECT_EQ(some(20), dht.getHumidity());
    EXPECT_EQ(some(420), dht.getTemperature());
    EXPECT_EQ(DHTState::IDLE, dht.getState());
    EXPECT_FALSE(pin.isInterruptOn);
    EXPECT_FALSE(comparator.isInterruptOn);
}

TEST(DHTTest, dht22_reads_negative_temperatures) {
    MockComparator comparator;
    MockPin pin, power;
    MockRealTimer rt;

    auto dht = DHT22(pin, power, comparator, rt);
    EXPECT_FALSE(pin.isOutput);

    // after 1 second, pin should be pulled low for 18ms
    rt.advance(1_s);
    dht.loop();
    dht.measure();
    EXPECT_EQ(DHTState::SIGNALING, dht.getState());
    EXPECT_TRUE(pin.isOutput);
    EXPECT_FALSE(pin.high);

    // after 18ms, switch to input and wait for low sync pulse
    rt.advance(18_ms);
    dht.loop();
    EXPECT_FALSE(pin.isOutput);
    EXPECT_EQ(DHTState::SYNC_LOW, dht.getState());

    // low sync pulse should take 60..120us
    pin.high = false;
    comparator.advance();
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    pin.high = true;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    dht.loop();
    dht.loop();
    EXPECT_EQ(DHTState::SYNC_HIGH, dht.getState());

    // high sync pulse should take 60..120us
    pin.high = false;
    comparator.advance(80_us);
    EXPECT_TRUE(pin.isInterruptOn);
    invoke<MockPin::INT>(dht);
    dht.loop();
    EXPECT_EQ(DHTState::RECEIVING_LOW, dht.getState());

    auto sendTestBit = [&] (bool isOne) {
        pin.high = true;
        comparator.advance(71_us); // my DHT22 sends rather slow "low" pulses
        invoke<MockPin::INT>(dht);
        dht.loop();

        pin.high = false;
        if (isOne) {
            comparator.advance(70_us);
        } else {
            comparator.advance(30_us);
        }
        invoke<MockPin::INT>(dht);
        dht.loop();
    };

    auto sendTestByte = [&] (uint8_t data) {
        for (int8_t bit = 7; bit >= 0; bit--) {
            sendTestBit((data & (1 << bit)) != 0);
        }
    };

    sendTestByte(0b00000010);
    sendTestByte(0b10001100);
    sendTestByte(0b10000000);
    sendTestByte(0b01100101);
    sendTestByte(59);

    EXPECT_EQ(some(652), dht.getHumidity());
    EXPECT_EQ(some(-101), dht.getTemperature());
}

TEST(DHTTest, reports_none_during_bootup) {
	FAIL("pending");
}

}
