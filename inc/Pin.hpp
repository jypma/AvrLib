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
#include "Stream.hpp"
#include "ExternalInterrupt.hpp"

template <typename info>
class Pin {
public:
    void configureAsOutput() const {
        info::configureAsGPIO();
        *info::ddr |= info::bitmask;
    }

    void configureAsInputWithoutPullup() const {
        info::configureAsGPIO();
        *info::ddr &= ~info::bitmask;
        *info::port &= ~info::bitmask;
    }
    void configureAsInputWithPullup() const {
        *info::ddr &= ~info::bitmask;
        *info::port |= info::bitmask;
    }

    void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    void setHigh() const {
        *info::port |= info::bitmask;
    }

    void setLow() const {
        *info::port &= ~info::bitmask;
    }

    bool isHigh() const {
        return (*info::port()) & info::bitmask;
    }
};

template <typename pinInfo, typename usartInfo, uint8_t writeFifoCapacity>
class UsartTxPin: public Pin<pinInfo>, public UsartTx<usartInfo, writeFifoCapacity>, public Stream {
public:
    UsartTxPin(): UsartTx<usartInfo, writeFifoCapacity>(), Stream(&UsartTx<usartInfo, writeFifoCapacity>::write) {}
};

template <typename pinInfo, typename extInterruptInfo, ExtInterruptHandler &_interrupt>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo, _interrupt> {

};

template <uint8_t bit>
struct PinOnPortD {
    static constexpr volatile uint8_t *ddr = &DDRD;
    static constexpr volatile uint8_t *port = &PORTD;
    static constexpr uint8_t bitmask = _BV(bit);
};

template <uint8_t bit>
struct PinOnPortB {
    static constexpr volatile uint8_t *ddr = &DDRB;
    static constexpr volatile uint8_t *port = &PORTB;
    static constexpr uint8_t bitmask = _BV(bit);
};

struct GPIOPin {
    static inline void configureAsGPIO() {}
};

struct PinD0Info: public PinOnPortD<0> {
    static inline void configureAsGPIO() {
        UCSR0B &= ~_BV(RXEN0); // disable hardware USART receiver
    }
};

struct PinD1Info: public PinOnPortD<1> {
    static inline void configureAsGPIO() {
        UCSR0B &= ~_BV(TXEN0); // disable hardware USART transmitter
    }
};

struct SPIPin {
    static inline void configureAsGPIO() {
        SPCR &= ~_BV(SPE);     // disable hardware SPI
    }
};

struct PinD2Info: public PinOnPortD<2>, public GPIOPin {};
struct PinD3Info: public PinOnPortD<3>, public GPIOPin {};
struct PinD4Info: public PinOnPortD<4>, public GPIOPin {};
struct PinD5Info: public PinOnPortD<5>, public GPIOPin {};
struct PinD6Info: public PinOnPortD<6>, public GPIOPin {};
struct PinD7Info: public PinOnPortD<7>, public GPIOPin {};
struct PinD8Info: public PinOnPortB<0>, public GPIOPin {};
struct PinD9Info: public PinOnPortB<1>, public GPIOPin {};
struct PinD10Info: public PinOnPortB<2>, public GPIOPin {};
struct PinD11Info: public PinOnPortB<3>, public GPIOPin {};
struct PinD12Info: public PinOnPortB<4>, public GPIOPin {};
struct PinD13Info: public PinOnPortB<5>, public GPIOPin {};

typedef Pin<PinD0Info> PinD0;
template <uint8_t writeFifoCapacity = 16> using PinD1 = UsartTxPin<PinD1Info,Usart0Info,writeFifoCapacity>;
typedef ExtInterruptPin<PinD2Info,Int0Info,extInt0> PinD2;
typedef ExtInterruptPin<PinD3Info,Int1Info,extInt1> PinD3;
typedef Pin<PinD4Info> PinD4;
typedef Pin<PinD5Info> PinD5;
typedef Pin<PinD6Info> PinD6;
typedef Pin<PinD7Info> PinD7;
typedef Pin<PinD8Info> PinD8;
typedef Pin<PinD9Info> PinD9;
typedef Pin<PinD10Info> PinD10;
typedef Pin<PinD11Info> PinD11;
typedef Pin<PinD12Info> PinD12;
typedef Pin<PinD13Info> PinD13;


#endif /* INTERRUPT_H_ */
