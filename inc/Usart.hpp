/*
 * USART.hpp
 *
 *  Created on: Jan 7, 2015
 *      Author: jan
 */

#ifndef USART_HPP_
#define USART_HPP_

#include "Fifo.hpp"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

extern AbstractFifo *usart0writeFifo;
extern AbstractFifo *usart0readFifo;

ISR(USART_UDRE_vect);
ISR(USART_RXC_vect);

/**
 * Configures the USART to use the given baud rate, and configures both pins (0 and 1) as transmitter
 * and receiver, respectively. If only one of them is to be used for USART, you can call Pin.configureAsGPIO() after
 * this method on either of them.
 *
 * Also enables interrupts.
 */
template <typename info>
class Usart {
public:
    typedef info usart_info_t;

    Usart(uint32_t baud = 57600) {
        uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
        if (baud_setting > 4095) {
            baud_setting = (F_CPU / 8 / baud - 1) / 2;
            *info::ucsra = 0;
        } else {
            *info::ucsra = 1 << U2X0;
        }

        *info::ubrr = baud_setting;
        *info::ucsrb = (*info::ucsrb | _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0)) & ~_BV(UDRIE0);
        *info::ucsrc = _BV(UCSZ00) | _BV(UCSZ01);
    }
};

template<typename info, uint8_t Capacity>
class UsartFifo: public Fifo<Capacity> {
public:
    void uncheckedWrite(uint8_t b) {
        AtomicScope _;

        Fifo<Capacity>::uncheckedWrite(b);
        *info::ucsrb |= _BV(UDRIE0);
        // clear the TXC bit -- "can be cleared by writing a one to its bit location"
        *info::ucsra |= _BV(TXC0);
    }

    bool write(uint8_t b) {
        AtomicScope _;
        bool result = Fifo<Capacity>::write(b);
        *info::ucsrb |= _BV(UDRIE0);
        // clear the TXC bit -- "can be cleared by writing a one to its bit location"
        *info::ucsra |= _BV(TXC0);
        return result;
    }

    inline Streams::Writer<UsartFifo, Streams::BlockingWriteSemantics<UsartFifo>> out() {
        return Streams::Writer<UsartFifo, Streams::BlockingWriteSemantics<UsartFifo>>(*this);
    }
};

template <typename info, uint8_t writeFifoCapacity>
class UsartTx {
    typedef UsartFifo<info, writeFifoCapacity> fifo_t;
    fifo_t writeFifo;
public:
    UsartTx() {
        AtomicScope::SEI _;
        info::writeFifo = &writeFifo;
    }

    inline Streams::Writer<fifo_t, Streams::BlockingWriteSemantics<fifo_t>> out() {
        return writeFifo.out();
    }

    inline void write(char ch) {
        writeFifo.write(ch);
    }

    static void flush() {
        if (info::writeFifo != nullptr) {
            while (info::writeFifo->hasContent()) ;
        }

        while (*info::ucsrb & _BV(UDRIE0)) ;
        while ((*info::ucsra & _BV(TXC0)) == 0) ;
    }
};

template <typename info, uint8_t readFifoCapacity>
class UsartRx {
    Fifo<readFifoCapacity> readFifo;
public:
    UsartRx() {
        AtomicScope::SEI _;
        info::readFifo = &readFifo;
    }

    inline Streams::Reader<AbstractFifo> in() {
        return readFifo.in();
    }

    inline void clear() {
        readFifo.clear();
    }

    inline void uncheckedRead(uint8_t &i) {
        readFifo.uncheckedRead(i);
    }

    inline uint8_t getReadAvailable() {
        return readFifo.getReadAvailable();
    }

    template <typename Proto>
    inline Streams::ReaderState expect() {
        return readFifo.expect<Proto>();
    }

    template <typename Proto, typename T>
    inline Streams::ReaderState readAs(T &t) {
        return readFifo.readAs<Proto,T>(t);
    }
};

struct Usart0Info {
    static constexpr volatile uint8_t *ucsra = &UCSR0A;
    static constexpr volatile uint8_t *ucsrb = &UCSR0B;
    static constexpr volatile uint8_t *ucsrc = &UCSR0C;
    static constexpr volatile uint16_t *ubrr = &UBRR0;
    static constexpr volatile uint8_t *udr = &UDR0;
    static constexpr AbstractFifo *&writeFifo = usart0writeFifo;
    static constexpr AbstractFifo *&readFifo = usart0readFifo;
};

typedef Usart<Usart0Info> Usart0;
template <uint8_t writeFifoCapacity = 16> using Usart0Tx = UsartTx<Usart0Info, writeFifoCapacity>;

#endif /* USART_HPP_ */
