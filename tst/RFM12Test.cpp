#include <gtest/gtest.h>
#include "HopeRF/RFM12.hpp"

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
        if (rx.read(r)) {
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
    typedef HAL::Atmel::InterruptVectors::VectorINT0_ INT;
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
    typedef HAL::Atmel::InterruptVectors::VectorTIMER0_COMPA_ INT;

    static constexpr uint8_t prescalerPower2 = 8;
    typedef uint8_t value_t;
    value_t target = 0;

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
    FS20Packet packet(0,0,0,0,0);
    rfm.out_fs20(packet);

    // pretend we have a packet available, that's how sending gets triggered
    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // header = 0
    spi.tx.clear();
    decltype(rfm)::onInterruptHandler::invoke(rfm);

    /*
    std::cout << "oh: ";
    while (spi.tx.hasContent()) {
        uint8_t b;
        spi.tx.read(b);
        std::cout << int(b) << ",";
    }
    std::cout << std::endl;
    */

    EXPECT_EQ(3, spi.tx.getSize());
    EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<0,0,0>>>>()));
    EXPECT_EQ(RFM12Mode::RECEIVING, rfm.getMode());

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // length = 0, so 2 extra bytes for CRC
    decltype(rfm)::onInterruptHandler::invoke(rfm);

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // 1st CRC byte
    decltype(rfm)::onInterruptHandler::invoke(rfm);

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // 2nd CRC byte
    spi.tx.clear();
    decltype(rfm)::onInterruptHandler::invoke(rfm);
    EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
    // 3x 0 from the status code and byte retrieval, then 2x IDLE mode and finally turn on TX.
    EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<0,0,0,130,13,130,13,130,61>>>>()));
    bool on = true;

    for (int pulse = 0; pulse < 141; pulse++) {
        decltype(rfm)::onComparatorHandler::invoke(rfm);
        EXPECT_EQ(RFM12Mode::SENDING_OOK, rfm.getMode());
        on = !on;
        if (on) {
            EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<130,61>>>>())); // TX on
        } else {
            EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<130,13>>>>())); // idle
        }
    }

    spi.tx.clear();
    decltype(rfm)::onComparatorHandler::invoke(rfm);
    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<184,0,130,13,130,221>>>>())); // Empty TX reg, Idle, Turn on RX

    // pretend we have a (real) byte available
    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // header = 0
    spi.tx.clear();
    decltype(rfm)::onInterruptHandler::invoke(rfm);
    EXPECT_EQ(3, spi.tx.getSize());
    EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<0,0,0>>>>()));
    EXPECT_EQ(RFM12Mode::RECEIVING, rfm.getMode());

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(0); // length = 0, so 2 extra bytes for CRC
    decltype(rfm)::onInterruptHandler::invoke(rfm);

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(97); // 1st CRC byte
    decltype(rfm)::onInterruptHandler::invoke(rfm);

    spi.rx.write(1 << 7);
    spi.rx.write(0);
    spi.rx.write(193); // 2nd CRC byte
    spi.tx.clear();
    decltype(rfm)::onInterruptHandler::invoke(rfm);
    EXPECT_EQ(RFM12Mode::LISTENING, rfm.getMode());
    EXPECT_TRUE((spi.tx.in().expect<Seq<Token<typestring<0,0,0,130,13,130,221>>>>())); // Idle + turn on RX
    EXPECT_TRUE(rfm.hasContent());
    rfm.on([] (auto in) {
        EXPECT_EQ(1, in.getReadAvailable()); // header
    });
}

}
