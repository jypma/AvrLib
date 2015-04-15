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

/**
 * comparator_t comparator    : The NonPWMTimerComparator to use as clock source
 * fifo_t fifo                : The Fifo-like source to read bytes from. Must have .remove() and .isReading().
 * target_t target            : Target, or Pin, to apply output on. Must have setHigh(bool).
 * callback_t callback        : Callback on which onSerialTxComplete() is invoked whenever we're done reading from the fifo.
 *                              TODO template specialization when target is in fact the output pin of comparator_t.
 */
template <typename comparator_t, comparator_t &comparator, typename fifo_t, fifo_t &fifo, typename target_t, target_t &target, typename callback_t, callback_t &callback>
class SerialTx {
    typedef typename comparator_t::value_t count_t;
    typedef SerialTx<comparator_t,comparator,fifo_t,fifo,target_t,target,callback_t,callback> This;

public:
    struct Config {
        uint8_t *prefix;
        uint8_t prefix_bits;

        count_t zero_a_duration;
        bool zero_a_high;
        count_t zero_b_duration;
        bool zero_b_high;
        count_t one_a_duration;
        bool one_a_high;
        count_t one_b_duration;
        bool one_b_high;

        bool parity;   /* whether to conclude with an (even) parity bit */

        uint8_t *postfix;
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


private:
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
    Config *currentConfig = nullptr;
    count_t lastPulseEnd = 0;

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
        if (fifo.isReading()) {
            fifo.remove(currentByte);
            bitState = BitState::DATA;
        } else {
            bitState = (currentConfig->parity) ? BitState::PARITY_BIT :
                       (currentConfig->postfix_bits > 0) ? BitState::POSTFIX : BitState::IDLE;
        }
    }

    void applyCurrentPulse() {
        bool high;
        count_t duration;

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

        target.setHigh(high);
        lastPulseEnd += duration;
        comparator.setTarget(lastPulseEnd);
        comparator.interruptOn();
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

        if (bitState == BitState::IDLE) {
            callback.onSerialTxComplete();
        }
    }

    InterruptHandler onComparatorInt = { this, &This::onComparator };

public:
    SerialTx() {
        comparator.interrupt().attach(onComparatorInt);
    }

    ~SerialTx() {
        comparator.interrupt().detach();
    }

    void start(Config *config) {
        AtomicScope _;

        currentConfig = config;
        bitIndex = 0;
        if (currentConfig->prefix_bits > 0) {
            bitState = BitState::PREFIX;
        } else {
            nextDataByte();
        }

        if (bitState == BitState::IDLE) {
            callback.onSerialTxComplete();
        } else {
            currentBit = getCurrentBit();
            lastPulseEnd = comparator.getValue();
            applyCurrentPulse();
        }
    }
};



#endif /* SERIALTX_HPP_ */
