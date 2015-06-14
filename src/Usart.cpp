#include "Usart.hpp"

AbstractFifo *usart0writeFifo = nullptr;
AbstractFifo *usart0readFifo = nullptr;

ISR(USART_UDRE_vect)
{
    // clear the TXC bit -- "can be cleared by writing a one to its bit location"
    UCSR0A |= _BV(TXC0);

    uint8_t next;
    if (usart0writeFifo != nullptr && usart0writeFifo->fastread(next)) {
        // There is more data in the output buffer. Send the next byte
        UDR0 = next;
    } else {
        // Buffer empty, so disable interrupts
        UCSR0B &= ~_BV(UDRIE0);
    }
}

ISR(USART_RX_vect) {
    if (usart0readFifo != nullptr) {
        uint8_t ch = UDR0;
        usart0readFifo->fastwrite(ch);
    }
}
