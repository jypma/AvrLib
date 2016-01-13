#ifndef HAL_ATMEL_USART_HPP_
#define HAL_ATMEL_USART_HPP_

#include <avr/io.h>
#include "Fifo.hpp"
#include "HAL/Atmel/InterruptVectors.hpp"

namespace HAL {
namespace Atmel {

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
protected:
    void onSendComplete() {
        // clear the TXC bit -- "can be cleared by writing a one to its bit location"
        *info::ucsra |= _BV(TXC0);

        uint8_t next;
        if (writeFifo.fastread(next)) {
            // There is more data in the output buffer. Send the next byte
            *info::udr = next;
        } else {
            // Buffer empty, so disable interrupts
            *info::ucsrb &= ~_BV(UDRIE0);
        }
    }

public:
    UsartTx() {
        AtomicScope::SEI _;
    }

    inline Streams::Writer<fifo_t, Streams::BlockingWriteSemantics<fifo_t>> out() {
        return writeFifo.out();
    }

    inline void write(char ch) {
        writeFifo.write(ch);
    }

    void flush() {
        if ((SREG & (1 << SREG_I)) > 0) {
            while (writeFifo.hasContent()) ;
        }

        while (*info::ucsrb & _BV(UDRIE0)) ;
        while ((*info::ucsra & _BV(TXC0)) == 0) ;
    }
};

template <typename info, uint8_t readFifoCapacity>
class UsartRx {
    Fifo<readFifoCapacity> readFifo;
protected:
    inline void onReceive() {
        uint8_t ch = *info::udr;
        readFifo.fastwrite(ch);
    }

public:
    UsartRx() {
        AtomicScope::SEI _;
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

} // namespace AVR
} // namespace HAL

#endif /* HAL_ATMEL_USART_HPP_ */
