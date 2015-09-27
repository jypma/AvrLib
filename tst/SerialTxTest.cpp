#include <gtest/gtest.h>
#include "Serial/SerialTx.hpp"
#include "Serial/RS232.hpp"

namespace SerialTxTest {

using namespace Serial;

struct MockComparator {
    typedef HAL::Atmel::InterruptVectors::VectorTIMER0_COMPA_ INT;
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
};

typedef MockComparator::value_t count_t;

struct MockPin {
    bool high = false;
    void setHigh (bool h) {
        high = h;
    }
};

template <typename tx_t>
uint8_t expect_zero(tx_t &tx, uint8_t start, MockComparator &comparator, MockPin &pin) {
    std::cout << "expect zero invoke A" << std::endl;
    EXPECT_EQ(uint8_t(start + 10), comparator.target);
    EXPECT_FALSE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    std::cout << "expect zero invoke B" << std::endl;
    EXPECT_EQ(uint8_t(start + 10 + 20), comparator.target);
    EXPECT_TRUE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    return start + 10 + 20;
}

template <typename tx_t>
uint8_t expect_one(tx_t &tx, uint8_t start, MockComparator &comparator, MockPin &pin) {
    std::cout << "expect one invoke A" << std::endl;
    EXPECT_EQ(uint8_t(start + 30), comparator.target);
    EXPECT_TRUE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    std::cout << "expect one invoke B" << std::endl;
    EXPECT_EQ(uint8_t(start + 30 + 40), comparator.target);
    EXPECT_FALSE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    return start + 30 + 40;
}

template <typename comparator_t, typename pin_t, typename tx_t>
uint8_t expectByte(tx_t &tx, SerialConfig &config, comparator_t &comparator, pin_t &pin, uint8_t t, uint8_t value) {
    if (config.bitOrder == SerialBitOrder::LSB_FIRST) {
        for (int bit = 0; bit < 8; bit++) {
            if (((value >> bit) & 1) != 0) {
                t = expect_one(tx, t, comparator, pin);
            } else {
                t = expect_zero(tx, t, comparator, pin);
            }
        }
    } else {
        for (int bit = 0; bit < 8; bit++) {
            if (((value << bit) & 128) != 0) {
                t = expect_one(tx, t, comparator, pin);
            } else {
                t = expect_zero(tx, t, comparator, pin);
            }
        }
    }
    return t;
}

template <typename fifo_t, typename comparator_t, typename pin_t, typename tx_t>
void transmitTestBytes(SerialConfig &config, uint8_t byte1, uint8_t byte2, fifo_t &fifo, comparator_t &comparator, pin_t &pin, tx_t &tx) {
    tx.sendFromSource();
    EXPECT_TRUE(tx.isSending());

    uint8_t t = 5;
    // ---- prefix: 001, value is sent in reverse, LSB first ----------------
    t = expect_zero(tx, t, comparator, pin);
    t = expect_zero(tx, t, comparator, pin);
    t = expect_one(tx, t, comparator, pin);

    // ---- value is sent in reverse, LSB first -----------
    t = expectByte(tx, config, comparator, pin, t, byte1);

    // parity
    if (parity_even_bit(byte1) == 1) {
        t = expect_one(tx, t, comparator, pin);
    } else {
        t = expect_zero(tx, t, comparator, pin);
    }

    if (byte2 != 0) {
        t = expectByte(tx, config, comparator, pin, t, byte2);

        // parity
        if (parity_even_bit(byte2) == 1) {
            t = expect_one(tx, t, comparator, pin);
        } else {
            t = expect_zero(tx, t, comparator, pin);
        }
    }
    EXPECT_TRUE(tx.isSending());

    // postfix: 0
    t = expect_zero(tx, t, comparator, pin);
    EXPECT_FALSE(tx.isSending());
    EXPECT_FALSE(fifo.isReading());
}

TEST(SerialTxTest, all_config_parts_are_transmitted_for_two_byte_messages) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    uint8_t prefix[] = { 0b100 };
    uint8_t postfix[] = { 0 };
    SerialConfig config = { false, prefix, 3, {false, 10}, {true, 20}, {true, 30}, {false, 40}, SerialParity::EVEN, SerialBitOrder::LSB_FIRST, postfix, 1 };
    ChunkPulseSource source = { fifo };
    MockComparator comparator;
    MockPin pin;
    auto tx = pulseTx(comparator, pin, source);

    fifo.out() << &config << uint8_t(42) << uint8_t(24);
    fifo.out() << &config << uint8_t(43) << uint8_t(34);
    transmitTestBytes(config, 42, 24, fifo, comparator, pin, tx);
    transmitTestBytes(config, 43, 34, fifo, comparator, pin, tx);
}

TEST(SerialTxTest, all_config_parts_are_transmitted_for_single_byte_messages) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    uint8_t prefix[] = { 0b100 };
    uint8_t postfix[] = { 0 };
    SerialConfig config = { false, prefix, 3, {false, 10}, {true, 20}, {true, 30}, {false, 40}, SerialParity::EVEN, SerialBitOrder::LSB_FIRST, postfix, 1 };
    ChunkPulseSource source = { fifo };
    MockComparator comparator;
    MockPin pin;
    auto tx = pulseTx(comparator, pin, source);

    fifo.out() << &config << uint8_t(42);
    fifo.out() << &config << uint8_t(43);
    transmitTestBytes(config, 42, 0, fifo, comparator, pin, tx);
    transmitTestBytes(config, 43, 0, fifo, comparator, pin, tx);
}

TEST(SerialTxTest, msb_first_messages_are_transmitted_correctly) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    uint8_t prefix[] = { 0b100 };
    uint8_t postfix[] = { 0 };
    SerialConfig config = { false, prefix, 3, {false, 10}, {true, 20}, {true, 30}, {false, 40}, SerialParity::EVEN, SerialBitOrder::MSB_FIRST, postfix, 1 };
    ChunkPulseSource source = { fifo };
    MockComparator comparator;
    MockPin pin;
    auto tx = pulseTx(comparator, pin, source);

    fifo.out() << &config << uint8_t(42) << uint8_t(24);
    fifo.out() << &config << uint8_t(43) << uint8_t(34);
    transmitTestBytes(config, 42, 24, fifo, comparator, pin, tx);
    transmitTestBytes(config, 43, 34, fifo, comparator, pin, tx);
}

TEST(SerialTxTest, startSend_with_empty_config_and_empty_packet_causes_no_side_effect) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    SerialConfig config = { false, nullptr, 0, {false, 10}, {true, 20}, {true, 30}, {false, 40}, SerialParity::NONE, SerialBitOrder::MSB_FIRST, nullptr, 0 };
    ChunkPulseSource source = { fifo };
    MockComparator comparator;
    MockPin pin;
    auto tx = pulseTx(comparator, pin, source);

    fifo.out() << &config;
    tx.sendFromSource();

    EXPECT_EQ(0, comparator.target);
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(tx.isSending());
}

template<typename tx_t>
uint8_t expect_rs232_zero(tx_t &tx, uint8_t start, MockComparator &comparator, MockPin &pin) {
    EXPECT_EQ(uint8_t(start + 10), comparator.target);
    EXPECT_FALSE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    return start + 10;
}

template<typename tx_t>
uint8_t expect_rs232_one(tx_t &tx, uint8_t start, MockComparator &comparator, MockPin &pin) {
    EXPECT_EQ(uint8_t(start + 10), comparator.target);
    EXPECT_TRUE(pin.high);
    tx_t::onComparatorHandler::invoke(tx);
    return start + 10;
}

template <typename tx_t, typename comparator_t, typename pin_t>
uint8_t expectRS232Byte(tx_t &tx, SerialConfig &config, comparator_t &comparator, pin_t &pin, uint8_t t, uint8_t value) {
        for (int bit = 0; bit < 8; bit++) {
            if (((value >> bit) & 1) != 0) {
                t = expect_rs232_one(tx, t, comparator, pin);
            } else {
                t = expect_rs232_zero(tx, t, comparator, pin);
            }
        }
    return t;
}

TEST(SerialTxTest, can_send_rs232_8n1_data) {
    Fifo<32> data;
    uint8_t postfix[] = { 0b1 };
    SerialConfig config = { false, nullptr, 0, {false, 10}, Pulse::empty(), {true, 10}, Pulse::empty(), SerialParity::NONE, SerialBitOrder::LSB_FIRST, postfix, 1 };
    MockComparator comparator;
    MockPin pin;
    StreamPulseSource source(data, config);
    auto tx = pulseTx(comparator, pin, source);

    data.out() << uint8_t(42) << uint8_t(24);
    tx.sendFromSource();

    uint8_t t = 5;

    t = expectRS232Byte(tx, config, comparator, pin, t, 42);
    t = expect_rs232_one(tx, t, comparator, pin);
    t = expectRS232Byte(tx, config, comparator, pin, t, 24);
    EXPECT_TRUE(tx.isSending());
    t = expect_rs232_one(tx, t, comparator, pin);

    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(tx.isSending());
}

}
