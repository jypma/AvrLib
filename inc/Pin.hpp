/*
 * Interrupt.h
 *
 *  Created on: Dec 14, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "gcc_type_traits.h"
#include "InterruptHandler.hpp"
#include "Usart.hpp"
#include "Writer.hpp"
#include "ExternalInterrupt.hpp"
#include "Timer.hpp"

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
    static void noop(void *delegate) {}
    static bool alwaysTrue(void *delegate) { return true; }
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
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::noop,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::noop,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::noop,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::write,
        &UsartTxPin<pinInfo,usartInfo,writeFifoCapacity>::alwaysTrue };


template <typename pinInfo, typename extInterruptInfo, InterruptChain &_interrupt>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo, _interrupt> {

};


ISR(PCINT1_vect);
void _enablePCIE1();
void _disablePCIE1IfNeeded();

extern InterruptChain _pcint8_handler;
extern InterruptChain _pcint9_handler;
extern InterruptChain _pcint10_handler;
extern InterruptChain _pcint11_handler;
extern InterruptChain _pcint12_handler;
extern InterruptChain _pcint13_handler;
extern uint8_t pci1_directional;
extern uint8_t pci1_rising;

template <typename pinInfo>
class ChangeInterruptPin: public Pin<pinInfo> {
public:
    InterruptChain &interrupt() {
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

struct NoTimer {};

template <typename pinInfo, typename timer_t, class Enable=void>
class PinOnComparatorA: public Pin<pinInfo> {
    typedef typename timer_t::fail error_wrong_timer_template_argument;
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorA<pinInfo, timer_t, typename std::enable_if<std::is_same<timer_t, NoTimer>::value>::type>: public Pin<pinInfo> {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorA<pinInfo, timer_t, typename std::enable_if<std::is_same<typename timer_t::timer_info_t, typename pinInfo::timer_info_t>::value>::type>: public Pin<pinInfo> {
    timer_t *t;
public:
    typedef typename timer_t::comparatorA_t comparator_t;
    inline PinOnComparatorA(timer_t &_timer): t(&_timer) {}
    inline comparator_t &timerComparator() const {
        return t->comparatorA();
    }
    inline timer_t &timer() const {
        return *t;
    }
};

template <typename pinInfo, typename timer_t, class Enable=void>
class PinOnComparatorB: public Pin<pinInfo> {
    typedef typename timer_t::fail error_wrong_timer_template_argument;
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorB<pinInfo, timer_t, typename std::enable_if<std::is_same<timer_t, NoTimer>::value>::type>: public Pin<pinInfo> {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorB<pinInfo, timer_t, typename std::enable_if<std::is_same<typename timer_t::timer_info_t, typename pinInfo::timer_info_t>::value>::type>: public Pin<pinInfo> {
    timer_t *t;
public:
    typedef typename timer_t::comparatorB_t comparator_t;
    inline PinOnComparatorB(timer_t &_timer): t(&_timer) {}
    inline comparator_t &timerComparator() const {
        return t->comparatorB();
    }
    inline timer_t &timer() const {
        return *t;
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

struct PinOnTimer0 {
    typedef Timer0Info timer_info_t;
};

struct PinOnTimer1 {
    typedef Timer1Info timer_info_t;
};

struct PinOnTimer2 {
    typedef Timer2Info timer_info_t;
};

struct PinD2Info: public PinOnPortD<2>, public GPIOPin {};
struct PinD3Info: public PinOnPortD<3>, public GPIOPin, public PinOnTimer2 {};
struct PinD4Info: public PinOnPortD<4>, public GPIOPin {};
struct PinD5Info: public PinOnPortD<5>, public GPIOPin, public PinOnTimer0 {};
struct PinD6Info: public PinOnPortD<6>, public GPIOPin, public PinOnTimer0 {};
struct PinD7Info: public PinOnPortD<7>, public GPIOPin {};
struct PinD8Info: public PinOnPortB<0>, public GPIOPin {};
struct PinD9Info: public PinOnPortB<1>, public GPIOPin, public PinOnTimer1 {};
struct PinD10Info: public PinOnPortB<2>, public GPIOPin, public PinOnTimer1 {};
struct PinD11Info: public PinOnPortB<3>, public GPIOPin, public PinOnTimer2 {};
struct PinD12Info: public PinOnPortB<4>, public GPIOPin {};
struct PinD13Info: public PinOnPortB<5>, public GPIOPin {};
struct PinA0Info: public PinOnPortC<0>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint8_handler;
};
struct PinA1Info: public PinOnPortC<1>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint9_handler;
};
struct PinA2Info: public PinOnPortC<2>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint10_handler;
};
struct PinA3Info: public PinOnPortC<3>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint11_handler;
};
struct PinA4Info: public PinOnPortC<4>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint12_handler;
};
struct PinA5Info: public PinOnPortC<5>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint13_handler;
};

typedef Pin<PinD0Info> PinD0;
template <uint8_t writeFifoCapacity = 16> using PinD1 = UsartTxPin<PinD1Info,Usart0Info,writeFifoCapacity>;
typedef ExtInterruptPin<PinD2Info,Int0Info,extInt0> PinD2;
template <typename timer2_t> class PinD3: public PinOnComparatorB<PinD3Info,timer2_t>, public ExtInterrupt<Int1Info,extInt1> {
public:
    inline PinD3(timer2_t &_timer): PinOnComparatorB<PinD3Info,timer2_t>(_timer) {}
};
typedef Pin<PinD4Info> PinD4;
template <typename timer0_t = NoTimer> using PinD5 = PinOnComparatorB<PinD5Info,timer0_t>;
template <typename timer0_t = NoTimer> using PinD6 = PinOnComparatorA<PinD6Info,timer0_t>;
typedef Pin<PinD7Info> PinD7;
typedef Pin<PinD8Info> PinD8;
template <typename timer1_t = NoTimer> using PinD9 = PinOnComparatorA<PinD9Info,timer1_t>;
template <typename timer1_t = NoTimer> using PinD10 = PinOnComparatorB<PinD10Info,timer1_t>;
template <typename timer2_t = NoTimer> using PinD11 = PinOnComparatorA<PinD11Info,timer2_t>;
typedef Pin<PinD12Info> PinD12;
typedef Pin<PinD13Info> PinD13;
typedef ChangeInterruptPin<PinA0Info> PinA0;

#endif /* INTERRUPT_H_ */
