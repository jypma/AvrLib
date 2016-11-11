#ifndef HAL_ATMEL_USART_HPP_
#define HAL_ATMEL_USART_HPP_

#include <avr/io.h>
#include "Fifo.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"

namespace HAL {
namespace Atmel {

using namespace InterruptHandlers;

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

template <typename info, uint8_t writeFifoCapacity>
class UsartTx {
    typedef UsartTx<info, writeFifoCapacity> This;
    Fifo<writeFifoCapacity>  writeFifo;

    static void startWriting() {
        if ((*info::ucsrb & _BV(UDRIE0)) == 0) {
            *info::ucsrb |= _BV(UDRIE0);
            // clear the TXC bit -- "can be cleared by writing a one to its bit location"
            *info::ucsra |= _BV(TXC0);
        }
    }

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
    typedef On<This, Int_USART_UDRE_, &This::onSendComplete> Handlers;

    UsartTx() {
        AtomicScope::SEI _;
    }

    template <typename... types>
    bool write(types... args) {
        return writeOrBlock (args...);
    }

    template <typename... types>
    bool writeOrBlock(types... args) {
        writeFifo.template writeOrBlockWith<&This::startWriting>(args...);

        AtomicScope _;
        if (writeFifo.hasContent()) {
            startWriting();
        }

        return true;
    }

    template <typename... types>
    bool writeIfSpace(types... args) {
        bool result = writeFifo.writeIfSpace(args...);

        AtomicScope _;
        if (writeFifo.hasContent()) {
            startWriting();
        }

        return result;
    }

    void flush() {
        if ((SREG & (1 << SREG_I)) == 0) {
        	// interrupts disabled, so can't flush.
        	return;
        }

    	uint16_t counter = 65000;
        while (writeFifo.hasContent() && ((counter--) > 0)) ;
    	counter = 65000;
        while ((*info::ucsrb & _BV(UDRIE0)) && ((counter--) > 0)) ;
        counter = 65000;
        while (((*info::ucsra & _BV(TXC0)) == 0) && ((counter--) > 0)) ;
    }
};

template <typename info, uint8_t readFifoCapacity>
class UsartRx:
    public Streams::ReadingDelegate<UsartRx<info, readFifoCapacity>, AbstractFifo>
{
    typedef UsartRx<info, readFifoCapacity> This;

    Fifo<readFifoCapacity> readFifo;

    inline void onReceive() {
        uint8_t ch = *info::udr;
        readFifo.fastwrite(ch);
    }

public:
    typedef On<This, Int_USART_RX_, &This::onReceive> Handlers;

    UsartRx(): Streams::ReadingDelegate<UsartRx<info, readFifoCapacity>, AbstractFifo>(&readFifo) {
        AtomicScope::SEI _;
    }
};

} // namespace AVR
} // namespace HAL

#endif /* HAL_ATMEL_USART_HPP_ */
