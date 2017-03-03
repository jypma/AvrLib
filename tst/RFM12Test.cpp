#include <gtest/gtest.h>
#include "HopeRF/RFM12.hpp"
#include "invoke.hpp"

namespace RFM12Test {

using namespace HopeRF;
using namespace FS20;
using namespace irqus;

class MockSPIMaster {
public:
    Fifo<250> rx;
    Fifo<250> tx;
    void setClockPrescaler(SPIPrescaler p) const {

    }
    uint8_t transceive(const uint8_t b) {
        tx.write(b);
        uint8_t r;
        if (rx.read(&r)) {
            return r;
        } else {
            return 0;
        }
    }
    void send(uint8_t b) {
        tx.write(b);
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
public:
    typedef HAL::Atmel::Int_INT0_ INT;
    bool isInput = false;
    void configureAsInputWithPullup() {
        isInput = true;
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
    typedef HAL::Atmel::Int_TIMER0_COMPA_ INT;

    static constexpr uint8_t prescalerPower2 = 8;
    typedef uint8_t value_t;
    value_t target = 0;
    bool interruptFlag;

    void setTarget(value_t _target) {
        target = _target;
    }

    void interruptOn() {
        interruptFlag = true;
    }

    void interruptOff() {
        interruptFlag = false;
    }

    value_t getValue() {
        return 5;
    }

    template <uint32_t value, typename return_t>
    static constexpr return_t microseconds2counts() {
        return (return_t) value;
    }
};


TEST(RFM12Test, rfm12_can_send_FSK_after_OOK) {
    MockSPIMaster spi;
    MockSSPin ss_pin;
    MockIntPin int_pin;
    MockComparator comp;
    auto rfm = rfm12(spi, ss_pin, int_pin, comp, RFM12Band::_868Mhz);

    EXPECT_TRUE(ss_pin.isOutput);
    EXPECT_TRUE(int_pin.isInput);

    spi.tx.clear();
    FS20Packet packet(0,0,0,0,0);
    rfm.write_fs20(packet);

    EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
    EXPECT_TRUE(comp.interruptFlag);
    // 2x IDLE mode and finally turn on TX.
    EXPECT_TRUE(spi.tx.read(FB(130,13,130,13,130,61)));
    bool on = true;

    for (int pulse = 0; pulse < 141; pulse++) {
        EXPECT_TRUE(comp.interruptFlag);
        invoke<MockComparator::INT>(rfm);
        EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
        on = !on;
        if (on) {
            EXPECT_TRUE(spi.tx.read(FB(130,61))); // TX on
        } else {
            EXPECT_TRUE(spi.tx.read(FB(130,13))); // idle
        }
    }

    spi.tx.clear();
    EXPECT_TRUE(comp.interruptFlag);
    invoke<MockComparator::INT>(rfm);
    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_TRUE(spi.tx.read(FB(184,0,130,13,130,221))); // Empty TX reg, Idle, Turn on RX
    EXPECT_FALSE(comp.interruptFlag);

    // pretend we have a (real) byte available
    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // header = 0
    spi.tx.clear();
    invoke<MockIntPin::INT>(rfm);
    EXPECT_EQ(3, spi.tx.getSize());
    EXPECT_TRUE(spi.tx.read(FB(0,0,0)));
    EXPECT_EQ(RFM12Mode::RECEIVING, rfm.getMode());

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // length = 0, so 2 extra bytes for CRC
    invoke<MockIntPin::INT>(rfm);

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(97)); // 1st CRC byte
    invoke<MockIntPin::INT>(rfm);

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(193)); // 2nd CRC byte
    spi.tx.clear();
    invoke<MockIntPin::INT>(rfm);
    Logging::Log<Loggers::Main>::debug(dec(&spi.tx));

    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_TRUE(spi.tx.read(FB(0,0,0,130,13,130,221))); // Idle + turn on RX
    EXPECT_TRUE(rfm.in().hasContent());
    rfm.in().readStart();
    EXPECT_EQ(1, rfm.in().getReadAvailable()); // header
}

TEST(RFM12Test, rfm12_sends_queued_ook_after_receiving_completes) {
    MockSPIMaster spi;
    MockSSPin ss_pin;
    MockIntPin int_pin;
    MockComparator comp;
    auto rfm = rfm12(spi, ss_pin, int_pin, comp, RFM12Band::_868Mhz);

    EXPECT_TRUE(ss_pin.isOutput);
    EXPECT_TRUE(int_pin.isInput);

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // header = 0
    spi.tx.clear();
    invoke<MockIntPin::INT>(rfm);

    EXPECT_EQ(3, spi.tx.getSize());
    EXPECT_TRUE(spi.tx.read(FB(0,0,0)));
    EXPECT_EQ(RFM12Mode::RECEIVING, rfm.getMode());

    FS20Packet packet(0,0,0,0,0);
    rfm.write_fs20(packet);
    EXPECT_EQ(RFM12Mode::RECEIVING, rfm.getMode());

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // length = 0, so 2 extra bytes for CRC
    invoke<MockIntPin::INT>(rfm);

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // 1st CRC byte
    invoke<MockIntPin::INT>(rfm);

    spi.rx.write(uint8_t(1 << 7));
    spi.rx.write(uint8_t(0));
    spi.rx.write(uint8_t(0)); // 2nd CRC byte
    spi.tx.clear();
    invoke<MockIntPin::INT>(rfm);

    EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());

    // 3 zeros from the SPI status byte transfer, then 2x IDLE mode and finally turn on TX.
    EXPECT_TRUE(spi.tx.read(FB(0,0,0,130,13,130,13,130,61)));
    bool on = true;

    for (int pulse = 0; pulse < 141; pulse++) {
        invoke<MockComparator::INT>(rfm);
        EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
        on = !on;
        if (on) {
            EXPECT_TRUE(spi.tx.read(FB(130,61))); // TX on
        } else {
            EXPECT_TRUE(spi.tx.read(FB(130,13))); // idle
        }
    }

    spi.tx.clear();
    invoke<MockComparator::INT>(rfm);
    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_TRUE(spi.tx.read(FB(184,0,130,13,130,221))); // Empty TX reg, Idle, Turn on RX
}

TEST(RFM12Test, can_send_multiple_queued_ook_packets) {
    MockSPIMaster spi;
    MockSSPin ss_pin;
    MockIntPin int_pin;
    MockComparator comp;
    auto rfm = rfm12(spi, ss_pin, int_pin, comp, RFM12Band::_868Mhz);

    spi.tx.clear();
    FS20Packet packet(0,0,0,0,0);
    rfm.write_fs20(packet);
    rfm.write_fs20(packet);

    EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());

    // 2x IDLE mode and finally turn on TX.
    EXPECT_TRUE(spi.tx.read(FB(130,13,130,13,130,61)));

    for (int pulse = 0; pulse < 141; pulse++) {
        invoke<MockComparator::INT>(rfm);
        EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
    }

    spi.tx.clear();
    invoke<MockComparator::INT>(rfm);
    EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
    EXPECT_TRUE(comp.interruptFlag);
    // RF_TXREG_WRITE, idle, idle, turn on TX
    EXPECT_TRUE(spi.tx.read(FB(184,0,130,13,130,13,130,61)));

    for (int pulse = 0; pulse < 141; pulse++) {
        invoke<MockComparator::INT>(rfm);
        EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
    }

    spi.tx.clear();
    invoke<MockComparator::INT>(rfm);
    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_FALSE(comp.interruptFlag);
    EXPECT_TRUE(spi.tx.read(FB(184,0,130,13,130,221))); // Empty TX reg, Idle, Turn on RX
}

}
