/*
 * USART.hpp
 *
 *  Created on: Jan 7, 2015
 *      Author: jan
 */

#ifndef USART_HPP_
#define USART_HPP_

#include "Fifo.hpp"
#include "Interrupt.hpp"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

constexpr char endl = '\n';

extern AbstractFifo *usart0writeFifo;

ISR(USART_UDRE_vect);

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
    }
};

template <typename info, uint8_t writeFifoCapacity>
class UsartTx {
    Fifo<writeFifoCapacity> writeFifo;
public:
    UsartTx() {
        AtomicScope::SEI _;
        info::fifo = &writeFifo;
    }

    static void write (uint8_t c) {
        if (info::fifo == nullptr) return;
        while (info::fifo->isFull()) ;

        AtomicScope _;
        info::fifo->append(c);
        *info::ucsrb |= _BV(UDRIE0);
        // clear the TXC bit -- "can be cleared by writing a one to its bit location"
        *info::ucsra |= _BV(TXC0);
    }
};

struct Usart0Info {
    static constexpr volatile uint8_t *ucsra = &UCSR0A;
    static constexpr volatile uint8_t *ucsrb = &UCSR0B;
    static constexpr volatile uint16_t *ubrr = &UBRR0;
    static constexpr volatile uint8_t *udr = &UDR0;
    static constexpr AbstractFifo *&fifo = usart0writeFifo;
};

extern const Usart<Usart0Info> usart0;
template <uint8_t writeFifoCapacity = 16> using Usart0Tx = UsartTx<Usart0Info, writeFifoCapacity>;

#endif /* USART_HPP_ */
