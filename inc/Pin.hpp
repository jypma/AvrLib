/*
 * Interrupt.h
 *
 *  Created on: Dec 14, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "InterruptHandler.hpp"
#include "Usart.hpp"
#include "Writer.hpp"
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
        return (*info::pin & info::bitmask) != 0;
    }

    bool isLow() const {
        return (*info::pin & info::bitmask) == 0;
    }
};

template <typename pinInfo, typename usartInfo, uint8_t writeFifoCapacity>
class UsartTxPin: public Pin<pinInfo>, public UsartTx<usartInfo, writeFifoCapacity> {
    const static Writer::VTable writerVTable;
    static void writeStart(void *delegate) {}
    static void writeEnd(void *delegate) {}
    static bool write(void *delegate, uint8_t b) {
        // TODO move blocking behavior into Writer constructor
        UsartTx<usartInfo,writeFifoCapacity>::write(b);
        return true;
    }
public:
    UsartTxPin(): UsartTx<usartInfo, writeFifoCapacity>() {}
    Writer out() {
        return Writer(&writerVTable, this);
    }
};

template <typename pinInfo, typename usartInfo, uint8_t writeFifoCapacity>
const Writer::VTable UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::writerVTable = {
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::writeStart,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::writeEnd,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::write };


template <typename pinInfo, typename extInterruptInfo, InterruptHandler &_interrupt>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo, _interrupt> {

};


ISR(PCINT1_vect);
void _enablePCIE1();
void _disablePCIE1IfNeeded();

extern InterruptHandler _pcint8_handler;
extern InterruptHandler _pcint9_handler;
extern InterruptHandler _pcint10_handler;
extern InterruptHandler _pcint11_handler;
extern InterruptHandler _pcint12_handler;
extern InterruptHandler _pcint13_handler;
extern uint8_t pci1_directional;
extern uint8_t pci1_rising;

template <typename pinInfo>
class ChangeInterruptPin: public Pin<pinInfo> {
public:
    InterruptHandler &interrupt() {
        return *pinInfo::pinChange;
    }

    void interruptOnChange() {
        pci1_directional &= ~pinInfo::bitmask;
        _enablePCIE1();
        *pinInfo::pcmsk |= pinInfo::bitmask;
    }

    void interruptOnRising() {
        pci1_rising |= pinInfo::bitmask;
        pci1_directional |= pinInfo::bitmask;
        _enablePCIE1();
        *pinInfo::pcmsk |= pinInfo::bitmask;
    }

    void interruptOnFalling() {
        pci1_rising &= ~pinInfo::bitmask;
        pci1_directional |= pinInfo::bitmask;
        _enablePCIE1();
        *pinInfo::pcmsk |= pinInfo::bitmask;
    }

    void interruptOff() {
        *pinInfo::pcmsk &= ~pinInfo::bitmask;
        _disablePCIE1IfNeeded();
    }
};

template <uint8_t bit>
struct PinOnPortD {
    static constexpr volatile uint8_t *ddr = &DDRD;
    static constexpr volatile uint8_t *port = &PORTD;
    static constexpr volatile uint8_t *pin = &PIND;
    static constexpr uint8_t bitmask = _BV(bit);
};

template <uint8_t bit>
struct PinOnPortB {
    static constexpr volatile uint8_t *ddr = &DDRB;
    static constexpr volatile uint8_t *port = &PORTB;
    static constexpr volatile uint8_t *pin = &PINB;
    static constexpr uint8_t bitmask = _BV(bit);
};

template <uint8_t bit>
struct PinOnPortC {
    static constexpr volatile uint8_t *ddr = &DDRC;
    static constexpr volatile uint8_t *port = &PORTC;
    static constexpr volatile uint8_t *pin = &PINC;
    static constexpr volatile uint8_t *pcmsk = &PCMSK1;
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
struct PinA0Info: public PinOnPortC<0>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint8_handler;
};
struct PinA1Info: public PinOnPortC<1>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint9_handler;
};
struct PinA2Info: public PinOnPortC<2>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint10_handler;
};
struct PinA3Info: public PinOnPortC<3>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint11_handler;
};
struct PinA4Info: public PinOnPortC<4>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint12_handler;
};
struct PinA5Info: public PinOnPortC<5>, public GPIOPin {
    static constexpr InterruptHandler* pinChange = &_pcint13_handler;
};

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
typedef ChangeInterruptPin<PinA0Info> PinA0;

#endif /* INTERRUPT_H_ */
