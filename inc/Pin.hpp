/*
 * Interrupt.h
 *
 *  Created on: Dec 14, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "Interrupt.hpp"
#include "Usart.hpp"
#include <avr/pgmspace.h>

typedef volatile void (*Callback)();

struct PinInfo {
    volatile uint8_t * const _port;
    volatile uint8_t * const _ddr;
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

protected:
    inline uint8_t const pinNumber() const { return info - pinInfos; }

    void configureAsGPIO() const {
        switch(pinNumber()) {
          case 0: UCSR0B &= ~_BV(TXEN0); break;
          case 1: UCSR0B &= ~_BV(RXEN0); break;
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

ISR(INT0_vect);
ISR(INT1_vect);

class HwInterruptPin: public Pin, public InterruptHandler {
    void externalInterruptOn(uint8_t mode);

    friend void INT0_vect();
    friend void INT1_vect();

public:
    constexpr HwInterruptPin(const PinInfo * const _info): Pin(_info) {}

    /**
     * Invokes an attached interrupt handler whenever the pin is low. Works in all sleep modes.
     * You should call externalInterruptOff() from your handler, otherwise it will be
     * repeatedly invoked.
     */
    void externalInterruptOnLow() {
        externalInterruptOn(0);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin changes value. Only works when
     * the I/O clock is running.
     */
    void externalInterruptOnChange() {
        externalInterruptOn(1);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from low to high. Only works when
     * the I/O clock is running.
     */
    void externalInterruptOnRising() {
        externalInterruptOn(2);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from high to low. Only works when
     * the I/O clock is running.
     */
    void externalInterruptOnFalling() {
        externalInterruptOn(3);
    }

    /**
     * Disables raising any interrupts for this pin (but does not remove any registered interrupt handler).
     */
    void externalInterruptOff();
};

class SerialTxPin: public Pin {
    Usart * const usart;
public:
    SerialTxPin(const PinInfo * const _info, Usart &_usart): Pin(_info), usart(&_usart) {}

    void configureAsSerialTx(AbstractFifo &_fifo, uint32_t baud) const {
        usart->configure(_fifo, baud);
    }

    const SerialTxPin &operator << (char *string) const {
        if (string != nullptr) {
            char c = *string;
            while (c) {
                usart->write(c);
                string++;
                c = *string;
            }
        }

        return *this;
    }

    const SerialTxPin &operator << (char ch) const {
        usart->write(ch);
        return *this;
    }
};


const Pin pinD0(pinInfos + 0);
const SerialTxPin pinD1(pinInfos + 1, usart0);
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
