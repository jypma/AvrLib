#include "Pin.hpp"
#include <avr/interrupt.h>

ISR(INT0_vect) {
//    pinD2.invoke();
}

ISR(INT1_vect) {
//    pinD3.invoke();
}

/*
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
*/
