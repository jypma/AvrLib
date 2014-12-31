#include "Pins.hpp"
#include <avr/interrupt.h>

HwInterruptPin pinD2(pinInfos + 2);
HwInterruptPin pinD3(pinInfos + 3);

SIGNAL(INT0_vect) {
    pinD2.externalInterrupt().invoke();
}

SIGNAL(INT1_vect) {
    pinD3.externalInterrupt().invoke();
}

const PinInfo PROGMEM pinInfos[] = {
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 0 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 1 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 2 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 3 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 4 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 5 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 6 },
    { (uint16_t) &PORTD, (uint16_t) &DDRD, 1 << 7 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 0 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 1 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 2 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 3 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 4 },
    { (uint16_t) &PORTB, (uint16_t) &DDRB, 1 << 5 },
    { (uint16_t) &PORTC, (uint16_t) &DDRD, 1 << 0 },
    { (uint16_t) &PORTC, (uint16_t) &DDRC, 1 << 1 },
    { (uint16_t) &PORTC, (uint16_t) &DDRC, 1 << 2 },
    { (uint16_t) &PORTC, (uint16_t) &DDRC, 1 << 3 },
    { (uint16_t) &PORTC, (uint16_t) &DDRC, 1 << 4 },
    { (uint16_t) &PORTC, (uint16_t) &DDRC, 1 << 5 }
};

void Pin::configureAsOutput() const {
    ScopedNoInterrupts cli;
    configureAsGPIO();
    *ddr() |= bitmask();
}

void Pin::configureAsInputWithoutPullup() const {
    ScopedNoInterrupts cli;
    configureAsGPIO();
    *ddr() &= ~bitmask();
    *port() &= ~bitmask();
}

void Pin::configureAsInputWithPullup() const {
    ScopedNoInterrupts cli;
    configureAsGPIO();
    *ddr() &= ~bitmask();
    *port() |= bitmask();
}

void Pin::setHigh() const {
    ScopedNoInterrupts cli;
    *port() |= bitmask();
}

void Pin::setLow() const {
    ScopedNoInterrupts cli;
    *port() &= ~bitmask();
}

bool Pin::isHigh() const {
    return ((*port()) & bitmask());
}
