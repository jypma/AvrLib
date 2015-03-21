#include <gtest/gtest.h>
#include "RFM12.hpp"

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
    bool isLow() {
        return false;
    }
};

MockSPIMaster spi;
MockSSPin ss_pin;
MockIntPin int_pin;
Fifo<16> rx;
ChunkedFifo rxf(&rx);
Fifo<16> tx;
ChunkedFifo txf(&tx);

TEST(RFM12Test, rfm12_configures_pins_correctly) {
    RFM12<typeof spi, spi, txf, rxf, typeof ss_pin, ss_pin, typeof int_pin, int_pin> rfm(RFM12Band::_868Mhz);

    EXPECT_TRUE(ss_pin.isOutput);
    EXPECT_TRUE(int_pin.isInput);
}
