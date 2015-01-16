#include "Usart.hpp"

const Usart<Usart0Info> usart0;
AbstractFifo *usart0writeFifo = nullptr;

ISR(USART_UDRE_vect)
{
    if (usart0writeFifo == nullptr || usart0writeFifo->isEmpty()) {
        // Buffer empty, so disable interrupts
        // *usart0.info->ucsrb() &= ~_BV(UDRIE0);
        UCSR0B &= ~_BV(UDRIE0);
    } else {
        // There is more data in the output buffer. Send the next byte
        // *usart0.info->udr() = usart0.fifo->remove();
        UDR0 = usart0writeFifo->remove();
    }
}

