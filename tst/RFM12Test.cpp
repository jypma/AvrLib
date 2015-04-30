#include <gtest/gtest.h>
#include "RFM12.hpp"

namespace RFM12Test {

class MockSPIMaster {
public:
    void setClockPrescaler(SPIPrescaler p) const {

    }
    uint8_t transceive(uint8_t out) const {
        return 0;
    }
    void send(uint8_t out) const {

    }
};

class MockSSPin {
public:
    bool isOutput = false;
    void configureAsOutput() {
        isOutput = true;
    }
    void setHigh () const {
    }
    void setLow() const {

    }
};

class MockIntPin {
    InterruptChain i;
public:
    bool isInput = false;
    void configureAsInputWithPullup() {
        isInput = true;
    }
    InterruptChain &interrupt() {
        return i;
    }
    void interruptOnLow() {

    }
    void interruptOff() {

    }
    bool isLow() {
        return false;
    }
};

struct MockComparator {
    typedef uint8_t value_t;
    InterruptChain i;
    value_t target = 0;

    InterruptChain &interrupt() {
        return i;
    }

    void setTarget(value_t _target) {
        target = _target;
    }

    void interruptOn() {

    }

    void interruptOff() {

    }

    value_t getValue() {
        return 5;
    }

    template <uint32_t value>
    static constexpr value_t microseconds2counts() {
        return uint16_t(value);
    }
};

MockSPIMaster rfm_spi;
MockSSPin rfm_ss_pin;
MockIntPin rfm_int_pin;
MockComparator rfm_comparator;

TEST(RFM12Test, rfm12_configures_pins_correctly) {
    RFM12<typeof rfm_spi, rfm_spi, typeof rfm_ss_pin, rfm_ss_pin, typeof rfm_int_pin, rfm_int_pin, typeof rfm_comparator, rfm_comparator> rfm(RFM12Band::_868Mhz);

    EXPECT_TRUE(rfm_ss_pin.isOutput);
    EXPECT_TRUE(rfm_int_pin.isInput);
}

}
