/*
 * SerialTx.hpp
 *
 *  Created on: Apr 11, 2015
 *      Author: jan
 */

#ifndef SERIALTX_HPP_
#define SERIALTX_HPP_

#include <util/parity.h>
#include "InterruptHandler.hpp"
#include "AtomicScope.hpp"

struct SerialConfig {
    const uint8_t *prefix;
    uint8_t prefix_bits;

    uint16_t zero_a_duration;
    bool zero_a_high;
    uint16_t zero_b_duration;
    bool zero_b_high;
    uint16_t one_a_duration;
    bool one_a_high;
    uint16_t one_b_duration;
    bool one_b_high;

    bool parity;   /* whether to conclude with an (even) parity bit */

    const uint8_t *postfix;
    uint8_t postfix_bits;

    uint8_t prefixBit(uint8_t bitIndex) {
        uint8_t b = prefix[bitIndex / 8];
        uint8_t bit = bitIndex % 8;
        return (b >> bit) & 1;
    }

    uint8_t postfixBit(uint8_t bitIndex) {
        uint8_t b = postfix[bitIndex / 8];
        uint8_t bit = bitIndex % 8;
        return (b >> bit) & 1;
    }
};

struct SerialTxVTable {
    bool (*hasMoreBytes)(void *ctx);
    void (*readByte)(void *ctx, uint8_t &b);
    void (*applyPulse)(void *ctx, bool high, uint16_t duration);
};

class AbstractSerialTx {
protected:
    enum BitState: uint8_t {
        IDLE, PREFIX, DATA, PARITY_BIT, POSTFIX
    };
    enum PulseState: uint8_t {
        A, B
    };

    BitState bitState = PREFIX;
    uint8_t bitIndex = 0;
    uint8_t currentBit = 0;
    PulseState pulseState = A;
    uint8_t currentByte = 0;
    SerialConfig *currentConfig = nullptr;
    const SerialTxVTable *vtable;
    void *ctx;

    AbstractSerialTx(const SerialTxVTable *_vtable, void *_ctx): vtable(_vtable), ctx(_ctx) {}

    uint8_t getCurrentBit() {
        switch(bitState) {
          case PREFIX: return currentConfig->prefixBit(bitIndex);
          case DATA: return (currentByte >> bitIndex) & 1;
          case PARITY_BIT: return parity_even_bit(currentByte);
          case POSTFIX: return currentConfig->postfixBit(bitIndex);
          default: return 0;
        }
        return 0;
    }

    void nextDataByte() {
        bitIndex = 0;
        pulseState = A;
        if (vtable->hasMoreBytes(ctx)) {
            vtable->readByte(ctx, currentByte);
            bitState = BitState::DATA;
        } else {
            bitState = (currentConfig->parity) ? BitState::PARITY_BIT :
                       (currentConfig->postfix_bits > 0) ? BitState::POSTFIX : BitState::IDLE;
        }
    }

    void applyCurrentPulse() {
        bool high;
        uint16_t duration;

        if (pulseState == A) {
            if (currentBit == 1) {
                high = currentConfig->one_a_high;
                duration = currentConfig->one_a_duration;
            } else {
                high = currentConfig->zero_a_high;
                duration = currentConfig->zero_a_duration;
            }
        } else {
            if (currentBit == 1) {
                high = currentConfig->one_b_high;
                duration = currentConfig->one_b_duration;
            } else {
                high = currentConfig->zero_b_high;
                duration = currentConfig->zero_b_duration;
            }
        }

        vtable->applyPulse(ctx, high, duration);
    }

    void applyNextBit() {
        pulseState = A;
        switch(bitState) {
        case PREFIX:
            bitIndex++;
            if (bitIndex >= currentConfig->prefix_bits) {
                bitState = DATA;
                bitIndex = 0;
                nextDataByte();
            }
            break;

        case DATA:
            bitIndex++;
            if (bitIndex >= 8) {
                nextDataByte();
            }
            break;

        case PARITY_BIT:
            bitIndex = 0;
            bitState = (currentConfig->postfix_bits > 0) ? BitState::POSTFIX : BitState::IDLE;
            break;

        case POSTFIX:
            bitIndex++;
            if (bitIndex >= currentConfig->postfix_bits) {
                bitState = IDLE;
                bitIndex = 0;
            }
            break;

        case IDLE: return;
        }

        currentBit = getCurrentBit();
        applyCurrentPulse();
    }

    void onComparator() {
        if (pulseState == A) {
            pulseState = B;
            if ((currentBit == 0 && currentConfig->zero_b_duration > 0)
             || (currentBit == 1 && currentConfig->one_b_duration > 0)) {
                applyCurrentPulse();
            } else {
                applyNextBit();
            }
        } else {
            applyNextBit();
        }
    }

    void start(SerialConfig *config) {
        AtomicScope _;

        currentConfig = config;
        bitIndex = 0;
        if (currentConfig->prefix_bits > 0) {
            bitState = BitState::PREFIX;
        } else {
            nextDataByte();
        }

        if (bitState != BitState::IDLE) {
            currentBit = getCurrentBit();
            applyCurrentPulse();
        }
    }

};

/**
 * comparator_t comparator    : The NonPWMTimerComparator to use as clock source
 * fifo_t fifo                : The Fifo-like source to read bytes from. Must have .remove() and .isReading().
 * target_t target            : Target, or Pin, to apply output on. Must have setHigh(bool).
 * callback_t callback        : Callback on which onSerialTxComplete() is invoked whenever we're done reading from the fifo.
 *                              TODO template specialization when target is in fact the output pin of comparator_t.
 */
template <typename comparator_t, comparator_t &comparator, typename fifo_t, typename target_t, typename callback_t>
class SerialTx: public AbstractSerialTx {
    typedef typename comparator_t::value_t count_t;
    typedef SerialTx<comparator_t,comparator,fifo_t,target_t,callback_t> This;

    count_t lastPulseEnd = 0;
    fifo_t &fifo;
    target_t &target;
    callback_t &callback;

    static void applyPulse(void *ctx, bool high, uint16_t duration) {
        This *instance = (This*)ctx;
        instance->target.setHigh(high);
        instance->lastPulseEnd += duration;
        comparator.setTarget(instance->lastPulseEnd);
        comparator.interruptOn();
    }

    static bool hasMoreBytes(void *ctx) {
        This *instance = (This*)ctx;
        return instance->fifo.isReading();
    }

    static void readByte(void *ctx, uint8_t &b) {
        This *instance = (This*)ctx;
        instance->fifo.read(b);
    }

    static const SerialTxVTable vtable;

    void onComparator() {
        AbstractSerialTx::onComparator();

        if (bitState == BitState::IDLE) {
            callback.onSerialTxComplete();
        }
    }

    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    SerialTx(fifo_t &_fifo, target_t &_target, callback_t &_callback): AbstractSerialTx(&vtable, this), fifo(_fifo), target(_target), callback(_callback) {
        comparator.interrupt().attach(onComparatorInt);
    }

    ~SerialTx() {
        comparator.interrupt().detach();
    }

    void start(SerialConfig *config) {
        AtomicScope _;

        lastPulseEnd = comparator.getValue();
        AbstractSerialTx::start(config);

        if (bitState == BitState::IDLE) {
            callback.onSerialTxComplete();
        }
    }
};

template <typename comparator_t, comparator_t &comparator, typename fifo_t, typename target_t, typename callback_t>
const SerialTxVTable SerialTx<comparator_t, comparator, fifo_t, target_t, callback_t>::vtable = {
    &SerialTx<comparator_t, comparator, fifo_t, target_t, callback_t>::hasMoreBytes,
    &SerialTx<comparator_t, comparator, fifo_t, target_t, callback_t>::readByte,
    &SerialTx<comparator_t, comparator, fifo_t, target_t, callback_t>::applyPulse
};


#endif /* SERIALTX_HPP_ */
