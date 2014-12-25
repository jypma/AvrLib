/*
 * Interrupt.h
 *
 *  Created on: Dec 14, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

typedef volatile void (*Callback)();

struct PinInfo {
    volatile uint8_t * const port;
    volatile uint8_t * const ddr;
    uint8_t const bitmask;
};

extern volatile void _configurePin0AsGPIO();
extern volatile void _configurePin1AsGPIO();
extern volatile void _noop();

const PinInfo PROGMEM pinInfos[] = {
    { &PORTD, &DDRD, 1 << 0 },
    { &PORTD, &DDRD, 1 << 1 },
    { &PORTD, &DDRD, 1 << 2 },
    { &PORTD, &DDRD, 1 << 3 },
    { &PORTD, &DDRD, 1 << 4 },
    { &PORTD, &DDRD, 1 << 5 },
    { &PORTD, &DDRD, 1 << 6 },
    { &PORTD, &DDRD, 1 << 7 },
    { &PORTB, &DDRB, 1 << 0 },
    { &PORTB, &DDRB, 1 << 1 },
    { &PORTB, &DDRB, 1 << 2 },
    { &PORTB, &DDRB, 1 << 3 },
    { &PORTB, &DDRB, 1 << 4 },
    { &PORTB, &DDRB, 1 << 5 },
    { &PORTC, &DDRD, 1 << 0 },
    { &PORTC, &DDRC, 1 << 1 },
    { &PORTC, &DDRC, 1 << 2 },
    { &PORTC, &DDRC, 1 << 3 },
    { &PORTC, &DDRC, 1 << 4 },
    { &PORTC, &DDRC, 1 << 5 }
};

class Pin {
    const PinInfo * const info;
    inline volatile uint8_t * const port() const { return info->port; }
    inline volatile uint8_t * const ddr() const { return info->ddr; }
    inline uint8_t const bitmask() const { return info->bitmask; }
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

    void configureAsOutput() const {
        configureAsGPIO();
        *ddr() |= bitmask();
    }


    void configureAsInputWithoutPullup() const {
        configureAsGPIO();
        *ddr() &= ~bitmask();
        *port() &= ~bitmask();
    }

    void configureAsInputWithPullup() const {
        configureAsGPIO();
        *ddr() &= ~bitmask();
        *port() |= bitmask();
    }

    void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    inline void setHigh() const {
        *port() &= ~bitmask();
    }

    inline void setLow() const {
        *port() |= bitmask();
    }

    bool isHigh() const {
        return ((*port()) & bitmask());
    }
};

class Interrupt {
    Callback * const callback;
public:
    constexpr Interrupt(Callback *callback_): callback(callback_) {}

    void attach(Callback callback_) const {
        *callback = callback_;
    }
};

extern Callback int_0_callback;
extern Callback int_1_callback;

class SerialReceiver {
public:
    void configureAsSerialReceiver() {

    }
};

class Pin2: public Pin, Interrupt {
public:
    constexpr Pin2(): Pin(pinInfos + 2), Interrupt(&int_0_callback) {}
};

class Pin3: public Pin, Interrupt {
public:
    constexpr Pin3(): Pin(pinInfos + 3), Interrupt(&int_1_callback) {}
};

const Pin pinD0(pinInfos + 0);
const Pin pinD1(pinInfos + 1);
const Pin2 pinD2;
const Pin3 pinD3;
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
