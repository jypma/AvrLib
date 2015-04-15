#include <gtest/gtest.h>
#include "SerialTx.hpp"

struct SerialTxMockComparator {
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

    value_t getValue() {
        return 5;
    }
};

struct SerialTxMockFifo {
    int count = 1;

    bool isReading() {
        return count > 0;
    }

    void remove (uint8_t &value) {
        value = 42;
        count--;
    }
};

struct SerialTxMockTarget {
    bool high = false;
    void setHigh(bool _high) {
        high = _high;
    }
};

struct SerialTxMockCallback {
    bool complete = false;
    void onSerialTxComplete() {
        complete = true;
    }
};

SerialTxMockComparator stx_comparator;
SerialTxMockFifo stx_fifo;
SerialTxMockTarget stx_target;
SerialTxMockCallback stx_callback;
typedef SerialTx<typeof stx_comparator, stx_comparator, typeof stx_fifo, stx_fifo, typeof stx_target, stx_target, typeof stx_callback, stx_callback> tx_t;

uint8_t expect_zero(uint8_t start) {
    EXPECT_EQ(uint8_t(start + 10), stx_comparator.target);
    EXPECT_FALSE(stx_target.high);
    stx_comparator.i.invoke();
    EXPECT_EQ(uint8_t(start + 10 + 20), stx_comparator.target);
    EXPECT_TRUE(stx_target.high);
    stx_comparator.i.invoke();
    return start + 10 + 20;
}

uint8_t expect_one(uint8_t start) {
    EXPECT_EQ(uint8_t(start + 30), stx_comparator.target);
    EXPECT_TRUE(stx_target.high);
    stx_comparator.i.invoke();
    EXPECT_EQ(uint8_t(start + 30 + 40), stx_comparator.target);
    EXPECT_FALSE(stx_target.high);
    stx_comparator.i.invoke();
    return start + 30 + 40;
}

TEST(SerialTxTest, all_config_parts_are_transmitted) {
    stx_comparator.target = 0;
    stx_target.high = false;
    stx_callback.complete = false;

    tx_t tx;
    uint8_t prefix[] = { 1 };
    uint8_t postfix[] = { 0 };
    tx_t::Config config = { prefix, 1, 10, false, 20, true, 30, true, 40, false, true, postfix, 1 };

    tx.start(&config);

    uint8_t t = 5;
    // ---- prefix: 1 ----------------
    t = expect_one(t);

    // ---- 42:  0b00101010 (but sent in reverse, LSB first) -----------
    t = expect_zero(t);
    t = expect_one(t);
    t = expect_zero(t);
    t = expect_one(t);
    t = expect_zero(t);
    t = expect_one(t);
    t = expect_zero(t);
    t = expect_zero(t);

    // parity: 1
    t = expect_one(t);

    // postfix: 0
    EXPECT_FALSE(stx_callback.complete);
    t = expect_zero(t);
    EXPECT_TRUE(stx_callback.complete);
}

TEST(SerialTxTest, empty_config_and_fifo_causes_callback_invocation_directly_with_no_interrupts) {
    stx_comparator.target = 0;
    stx_target.high = false;
    stx_callback.complete = false;

    tx_t tx;
    tx_t::Config config = { nullptr, 0, 10, false, 20, true, 30, true, 40, false, false, nullptr, 0 };
    tx.start(&config);
    EXPECT_EQ(0, stx_comparator.target);
    EXPECT_FALSE(stx_target.high);
    EXPECT_TRUE(stx_callback.complete);
}
