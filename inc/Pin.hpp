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
#include "ExternalInterrupt.hpp"
#include "Timer.hpp"

template <typename info>
class Pin {
public:
    typedef info info_t;

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

struct NoUsart {};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity, class Enable=void>
class UsartTxPin: public Pin<pinInfo> {
    typedef typename usart_t::fail error_wrong_usart_template_argument;
};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity>
class UsartTxPin<pinInfo, usart_t, writeFifoCapacity, typename std::enable_if<std::is_same<usart_t, NoUsart>::value>::type>: public Pin<pinInfo> {
    // If NoUsart is provided as usart_t, the pin will be defined without usart capability.
};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity>
class UsartTxPin<pinInfo, usart_t, writeFifoCapacity, typename std::enable_if<std::is_same<typename usart_t::usart_info_t, typename pinInfo::usart_info_t>::value>::type>:
  public Pin<pinInfo>, public UsartTx<typename usart_t::usart_info_t, writeFifoCapacity> {
  public:
      UsartTxPin(const Usart<typename usart_t::usart_info_t> &usart) {}
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity, class Enable=void>
class UsartRxPin: public Pin<pinInfo> {
    typedef typename usart_t::fail error_wrong_usart_template_argument;
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity>
class UsartRxPin<pinInfo, usart_t, readFifoCapacity, typename std::enable_if<std::is_same<usart_t, NoUsart>::value>::type>: public Pin<pinInfo> {
    // If NoUsart is provided as usart_t, the pin will be defined without usart capability.
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity>
class UsartRxPin<pinInfo, usart_t, readFifoCapacity, typename std::enable_if<std::is_same<typename usart_t::usart_info_t, typename pinInfo::usart_info_t>::value>::type>:
  public Pin<pinInfo>, public UsartRx<typename usart_t::usart_info_t, readFifoCapacity> {
  public:
      UsartRxPin(const Usart<typename usart_t::usart_info_t> &usart) {}
};

template <typename pinInfo, typename extInterruptInfo, InterruptChain &_interrupt>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo, _interrupt> {};


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
class PinOnComparatorB<pinInfo, timer_t, typename std::enable_if<std::is_same<typename timer_t::timer_info_t, typename pinInfo::timer_info_t>::value>::type>:
    public Pin<pinInfo>,
    public Prescaled<typename timer_t::value_t, typename timer_t::prescaler_t, timer_t::prescaler> {
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

    void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    void setHigh() const {
        *pinInfo::port |= pinInfo::bitmask;
        if (timerComparator().isOutputConnected()) {
            // If one of the timer comparator modes is enabled, the pin's port bit actually doesnt do anything.
            // Instead, we change output mode. This only works for non-PWM modes, but during PWM it doesn't make
            // sense to call setHigh() anyways.

            // TODO move this hack so it only is present on PinOnComparatorA/B that are linked to a NonPWMComparator
            typedef typename comparator_t::comparator_info_t info;
            *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(NonPWMOutputMode::high_on_match) << info::output_mode_bitstart);
            *info::tccrb |= (1 << info::foc);
        }
    }

    void setLow() const {
        *pinInfo::port &= ~pinInfo::bitmask;
        if (timerComparator().isOutputConnected()) {
            // If one of the timer comparator modes is enabled, the pin's port bit actually doesnt do anything.
            // Instead, we change output mode. This only works for non-PWM modes, but during PWM it doesn't make
            // sense to call setHigh() anyways.

            // TODO move this hack so it only is present on PinOnComparatorA/B that are linked to a NonPWMComparator
            typedef typename comparator_t::comparator_info_t info;
            *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(NonPWMOutputMode::low_on_match) << info::output_mode_bitstart);
            *info::tccrb |= (1 << info::foc);
        }
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
    typedef Usart0Info usart_info_t;

    static inline void configureAsGPIO() {
        UCSR0B &= ~_BV(RXEN0); // disable hardware USART receiver
    }
};

struct PinD1Info: public PinOnPortD<1> {
    typedef Usart0Info usart_info_t;

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
    static constexpr uint8_t adc_mux = 0;
};
struct PinA1Info: public PinOnPortC<1>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint9_handler;
    static constexpr uint8_t adc_mux = 1;
};
struct PinA2Info: public PinOnPortC<2>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint10_handler;
    static constexpr uint8_t adc_mux = 2;
};
struct PinA3Info: public PinOnPortC<3>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint11_handler;
    static constexpr uint8_t adc_mux = 3;
};
struct PinA4Info: public PinOnPortC<4>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint12_handler;
    static constexpr uint8_t adc_mux = 4;
};
struct PinA5Info: public PinOnPortC<5>, public GPIOPin {
    static constexpr InterruptChain* pinChange = &_pcint13_handler;
    static constexpr uint8_t adc_mux = 5;
};
struct PinA6Info {
    static constexpr uint8_t adc_mux = 6;
};
struct PinA7Info {
    static constexpr uint8_t adc_mux = 7;
};

template <typename pinInfo>
class ADCOnlyPin {
    typedef pinInfo info_t;
};

template <typename usart_t = NoUsart, uint8_t readFifoCapacity = 32> using PinD0 = UsartRxPin<PinD0Info, usart_t, readFifoCapacity>;
template <typename usart_t = NoUsart, uint8_t writeFifoCapacity = 16> using PinD1 = UsartTxPin<PinD1Info, usart_t, writeFifoCapacity>;
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
typedef ChangeInterruptPin<PinA1Info> PinA1;
typedef ChangeInterruptPin<PinA2Info> PinA2;
typedef ChangeInterruptPin<PinA3Info> PinA3;
typedef ChangeInterruptPin<PinA4Info> PinA4;
typedef ChangeInterruptPin<PinA5Info> PinA5;
typedef ADCOnlyPin<PinA6Info> PinA6;
typedef ADCOnlyPin<PinA7Info> PinA7;

#endif /* INTERRUPT_H_ */
