#include "Usart.hpp"

const Usart<Usart0Info> usart0;
AbstractFifo *usart0writeFifo = nullptr;

ISR(USART_UDRE_vect)
{
    if (usart0writeFifo == nullptr || usart0writeFifo->isEmpty()) {
        // Buffer empty, so disable interrupts
        UCSR0B &= ~_BV(UDRIE0);
    } else {
        // There is more data in the output buffer. Send the next byte
        uint8_t next;
        usart0writeFifo->read(next);
        UDR0 = next;
    }
}

