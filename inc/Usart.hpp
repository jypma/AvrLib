/*
 * USART.hpp
 *
 *  Created on: Jan 7, 2015
 *      Author: jan
 */

#ifndef USART_HPP_
#define USART_HPP_

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "Fifo.hpp"

struct UsartInfo {
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint16_t *_ubrr;
    volatile uint8_t *_udr;

    inline volatile uint8_t *ucsra() const {
        return (volatile uint8_t *)pgm_read_ptr(&_ucsra);
    }

    inline volatile uint8_t *ucsrb() const {
        return (volatile uint8_t *)pgm_read_ptr(&_ucsrb);
    }

    inline volatile uint16_t *ubrr() const {
        return (volatile uint16_t *)pgm_read_ptr(&_ubrr);
    }

    inline volatile uint8_t *udr() const {
        return (volatile uint8_t *)pgm_read_ptr(&_udr);
    }

};

extern const UsartInfo PROGMEM usartInfos[];
constexpr char endl = '\n';

ISR(USART_UDRE_vect);

class Usart {
    friend void USART_UDRE_vect();
protected:
    const UsartInfo * const info;
    AbstractFifo *fifo = nullptr;
public:
    Usart(const UsartInfo * const _info): info(_info) {}

    /**
     * Configures the USART to use the given baud rate, and configures both pins (0 and 1) as transmitter
     * and receiver, respectively. If only one of them is to be used for USART, you can call Pin.configureAsGPIO() after
     * this method on either of them.
     *
     * Also enables interrupts.
     */
    void configure(AbstractFifo &_fifo, uint32_t baud) {
        sei();
        ScopedNoInterrupts cli;

        fifo = &_fifo;
        uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
        if (baud_setting > 4095) {
            baud_setting = (F_CPU / 8 / baud - 1) / 2;
            *info->ucsra() = 0;
        } else {
            *info->ucsra() = 1 << U2X0;
        }

        *info->ubrr() = baud_setting;
        *info->ucsrb() = (*info->ucsrb() | _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0)) & ~_BV(UDRIE0);
    }

    void write (uint8_t c);
};

extern Usart usart0;

#endif /* USART_HPP_ */
