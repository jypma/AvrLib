#include <gtest/gtest.h>
#include "SerialTx.hpp"

namespace SerialTxTest {

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
};

typedef MockComparator::value_t count_t;

struct MockPin {
    bool high = false;
    void setHigh (bool h) {
        high = h;
    }
};

uint8_t expect_zero(uint8_t start, MockComparator &comparator, MockPin &pin) {
    EXPECT_EQ(uint8_t(start + 10), comparator.target);
    EXPECT_FALSE(pin.high);
    comparator.i.invoke();
    EXPECT_EQ(uint8_t(start + 10 + 20), comparator.target);
    EXPECT_TRUE(pin.high);
    comparator.i.invoke();
    return start + 10 + 20;
}

uint8_t expect_one(uint8_t start, MockComparator &comparator, MockPin &pin) {
    EXPECT_EQ(uint8_t(start + 30), comparator.target);
    EXPECT_TRUE(pin.high);
    comparator.i.invoke();
    EXPECT_EQ(uint8_t(start + 30 + 40), comparator.target);
    EXPECT_FALSE(pin.high);
    comparator.i.invoke();
    return start + 30 + 40;
}

template <typename fifo_t, typename comparator_t, typename pin_t, typename tx_t>
void transmitTestByte(SerialConfig &config, uint8_t value, fifo_t &fifo, comparator_t &comparator, pin_t &pin, tx_t &tx) {
    tx.sendFromSource();
    EXPECT_TRUE(tx.isSending());

    uint8_t t = 5;
    // ---- prefix: 1 ----------------
    t = expect_one(t, comparator, pin);

    // ---- value is sent in reverse, LSB first -----------
    for (int bit = 0; bit < 8; bit++) {
        if (((value >> bit) & 1) == 1) {
            t = expect_one(t, comparator, pin);
        } else {
            t = expect_zero(t, comparator, pin);
        }
    }

    // parity
    if (parity_even_bit(value) == 1) {
        t = expect_one(t, comparator, pin);
    } else {
        t = expect_zero(t, comparator, pin);
    }

    EXPECT_TRUE(tx.isSending());

    // postfix: 0
    t = expect_zero(t, comparator, pin);
    EXPECT_FALSE(tx.isSending());
    EXPECT_FALSE(fifo.isReading());
}

TEST(SerialTxTest, all_config_parts_are_transmitted) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    uint8_t prefix[] = { 1 };
    uint8_t postfix[] = { 0 };
    SerialConfig config = { false, prefix, 1, {false, 10}, {true, 20}, {true, 30}, {false, 40}, true, postfix, 1 };
    ChunkPulseSource source = { &fifo };
    std::cout << "source is at " << long(&source) << std::endl;
    std::cout << "should invoke " << (long)(void*)(&ChunkPulseSource::getNextPulse) << std::endl << std::flush;
    MockComparator comparator;
    MockPin pin;
    auto tx = SoftwarePulseTx<MockComparator, MockPin, ChunkPulseSource>(comparator, pin, source);

    fifo.out() << &config << uint8_t(42);
    fifo.out() << &config << uint8_t(43);
    transmitTestByte(config, 42, fifo, comparator, pin, tx);
    transmitTestByte(config, 43, fifo, comparator, pin, tx);
}

TEST(SerialTxTest, startSend_with_empty_config_and_empty_packet_causes_no_side_effect) {
    Fifo<32> data;
    ChunkedFifo fifo(&data);
    SerialConfig config = { false, nullptr, 0, {false, 10}, {true, 20}, {true, 30}, {false, 40}, false, nullptr, 0 };
    ChunkPulseSource source = { &fifo };
    MockComparator comparator;
    MockPin pin;
    auto tx = softwarePulseTx(comparator, pin, source);

    fifo.out() << &config;
    tx.sendFromSource();

    EXPECT_EQ(0, comparator.target);
    EXPECT_FALSE(pin.high);
    EXPECT_FALSE(tx.isSending());
}

}
