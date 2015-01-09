#include "Pin.hpp"
#include <avr/interrupt.h>

HwInterruptPin pinD2(pinInfos + 2);
HwInterruptPin pinD3(pinInfos + 3);

ISR(INT0_vect) {
    pinD2.invoke();
}

ISR(INT1_vect) {
    pinD3.invoke();
}

const PinInfo PROGMEM pinInfos[] = {
    { &PORTD, &DDRD, 1 << 0 },
    { &PORTD, &DDRD, 1 << 1 },
    { &PORTD, &DDRD, 1 << 2 },
    { &PORTD, &DDRD, 1 << 3 },
    { &PORTD, &DDRD, 1 << 4 },
    { &PORTD, &DDRD, 1 << 5 },
    { &PORTD, &DDRD, 1 << 6 },
    { &PORTD, &DDRD, 1 << 7 },
    { &PORTB, &DDRB, 1 << 0 },
    { &PORTB, &DDRB, 1 << 1 },
    { &PORTB, &DDRB, 1 << 2 },
    { &PORTB, &DDRB, 1 << 3 },
    { &PORTB, &DDRB, 1 << 4 },
    { &PORTB, &DDRB, 1 << 5 },
    { &PORTC, &DDRD, 1 << 0 },
    { &PORTC, &DDRC, 1 << 1 },
    { &PORTC, &DDRC, 1 << 2 },
    { &PORTC, &DDRC, 1 << 3 },
    { &PORTC, &DDRC, 1 << 4 },
    { &PORTC, &DDRC, 1 << 5 }
};

void Pin::configureAsOutput() const {
    //ScopedNoInterrupts cli;
    //configureAsGPIO();
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

void HwInterruptPin::externalInterruptOn(uint8_t mode) {
    switch (pinNumber()) {
        case 2:
            EICRA = (EICRA & ~(_BV(ISC00) | _BV(ISC01))) | (mode << ISC00);
            EIMSK |= _BV(INT0);
            break;
        case 3:
            EICRA = (EICRA & ~(_BV(ISC10) | _BV(ISC11))) | (mode << ISC10);
            EIMSK |= _BV(INT1);
            break;
    }
}

void HwInterruptPin::externalInterruptOff() {
    switch (pinNumber()) {
        case 2: EIMSK &= ~_BV(INT0); break;
        case 3: EIMSK &= ~_BV(INT1); break;
    }
}
