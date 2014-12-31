/*
 * Interrupt.h
 *
 *  Created on: Dec 14, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "Interrupt.hpp"
#include <avr/pgmspace.h>

typedef volatile void (*Callback)();

struct PinInfo {
    uint16_t const _port;
    uint16_t const _ddr;
    uint8_t const _bitmask;

    inline volatile uint8_t *port() const {
        return (volatile uint8_t *)pgm_read_ptr(&_port);
    }
    inline volatile uint8_t *ddr() const {
        return (volatile uint8_t *)pgm_read_ptr(&_ddr);
    }
    inline volatile uint8_t bitmask() const {
        return pgm_read_byte(&_bitmask);
    }
};

extern const PinInfo PROGMEM pinInfos[];

class Pin {
    const PinInfo * const info;
    inline volatile uint8_t * const port() const { return info->port(); }
    inline volatile uint8_t * const ddr() const { return info->ddr(); }
    inline uint8_t const bitmask() const { return info->bitmask(); }
    inline uint8_t const pinNumber() const { return info - pinInfos; }

protected:
    void configureAsGPIO() const {
        switch(pinNumber()) {
          case 0: UCSR0B &= ~(1 << TXEN0); break;
          case 1: UCSR0B &= ~(1 << RXEN0); break;
        }
    }

public:
    constexpr Pin(const PinInfo * const _info): info(_info) {}

    void configureAsOutput() const;
    void configureAsInputWithoutPullup() const;
    void configureAsInputWithPullup() const;

    inline void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    void setHigh() const;
    void setLow() const;
    bool isHigh() const;
};

class HwInterruptPin: public Pin {
    InterruptHandler extInterrupt;
public:
    constexpr HwInterruptPin(const PinInfo * const _info): Pin(_info) {}
    InterruptHandler &externalInterrupt() { return extInterrupt; }
};

const Pin pinD0(pinInfos + 0);
const Pin pinD1(pinInfos + 1);
extern HwInterruptPin pinD2;
extern HwInterruptPin pinD3;
const Pin pinD4(pinInfos + 4);
const Pin pinD5(pinInfos + 5);
const Pin pinD6(pinInfos + 6);
const Pin pinD7(pinInfos + 7);
const Pin pinD8(pinInfos + 8);
const Pin pinD9(pinInfos + 9);
const Pin pinD10(pinInfos + 10);
const Pin pinD11(pinInfos + 11);
const Pin pinD12(pinInfos + 12);
const Pin pinD13(pinInfos + 13);

#endif /* INTERRUPT_H_ */
