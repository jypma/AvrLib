#ifndef HAL_ATMEL_USART_HPP_
#define HAL_ATMEL_USART_HPP_

#include "Fifo.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "HAL/Atmel/Registers.hpp"

namespace HAL {
namespace Atmel {

using namespace InterruptHandlers;
using namespace Registers;

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
            info::U2X.clear();
        } else {
            info::U2X.set();
        }

        info::UBRR.val() = baud_setting;
        info::TXEN.set();
        info::RXEN.set();
        info::RXCIE.set();
        info::UDRIE.clear();
        info::UCSRC = info::UCSZ0 | info::UCSZ1;
    }
};

template <typename info, uint8_t writeFifoCapacity>
class UsartTx {
    typedef UsartTx<info, writeFifoCapacity> This;
    Fifo<writeFifoCapacity>  writeFifo = {};
    uint8_t available = 0;

    static void startWriting() {
        if (info::UDRIE.isCleared()) {
        	info::UDRIE.set();
            // clear the TXC bit -- "can be cleared by writing a one to its bit location"
        	info::TXC.set();
        }
    }

    __attribute__((always_inline)) inline void onSendComplete() {
        // clear the TXC bit -- "can be cleared by writing a one to its bit location"
    	info::TXC.set();

        if (available > 0 || ((available = writeFifo._getSize()) > 0)) {
            // There is more data in the output buffer. Send the next byte
            uint8_t next;
            writeFifo._uncheckedRead(next);
            info::UDR.val() = next;
            available--;
        } else {
			// Buffer empty, so disable interrupts
        	info::UDRIE.clear();
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
            available = writeFifo.getSize();
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

    void clear() {
        writeFifo.clear();
    }

    void flush() {
        if (SREG_I.isCleared()) {
        	// interrupts disabled, so can't flush.
        	return;
        }

    	uint16_t counter = 65000;
        while (writeFifo.hasContent() && ((counter--) > 0)) ;
    	counter = 65000;
        while (info::UDRIE.isSet() && ((counter--) > 0)) ;
        counter = 65000;
        while (info::TXC.isCleared() && ((counter--) > 0)) ;
    }
};

template <typename info, uint8_t readFifoCapacity>
class UsartRx:
    public Streams::Impl::ReadingDelegate<AbstractFifo>
{
    typedef UsartRx<info, readFifoCapacity> This;

    Fifo<readFifoCapacity> readFifo = {};

    inline void onReceive() {
        uint8_t ch = info::UDR.get();
        readFifo.fastwrite(ch);
    }

public:
    typedef On<This, Int_USART_RX_, &This::onReceive> Handlers;

    UsartRx(): Streams::Impl::ReadingDelegate<AbstractFifo>(&readFifo) {
        AtomicScope::SEI _;
    }
};

} // namespace AVR
} // namespace HAL

#endif /* HAL_ATMEL_USART_HPP_ */
