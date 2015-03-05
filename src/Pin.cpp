#include "Pin.hpp"

InterruptHandler _pcint8_handler;
InterruptHandler _pcint9_handler;
InterruptHandler _pcint10_handler;
InterruptHandler _pcint11_handler;
InterruptHandler _pcint12_handler;
InterruptHandler _pcint13_handler;

uint8_t lastPortC;

uint8_t pci1_directional; // whether or not a PCI1 handler is only on rising or falling (1) or all changes (0)
uint8_t pci1_rising;      // whether, for above == 1, to invoke on rising (1) or falling (0)

void _enablePCIE1() {
    if ((PCICR & _BV(PCIE1)) == 0) {
        PCICR |= _BV(PCIE1);
        lastPortC = PINC;
    }
}

void _disablePCIE1IfNeeded() {
    if ((PCICR & _BV(PCIE1)) != 0) {
        if (PCMSK1 == 0) { // no more handlers are registered
            PCICR &= ~_BV(PCIE1);
        }
    }
}

inline bool shouldInvokeFor(uint8_t current, uint8_t changed, uint8_t mask) {
    if (!(changed & mask)) return false;
    if (!(pci1_directional & mask)) return true;
    return (current & mask) == (pci1_rising & mask);
}

ISR(PCINT1_vect) {
    uint8_t current = PINC;
    uint8_t changed = current ^ lastPortC;
    if (shouldInvokeFor(current, changed, _BV(0))) {
        _pcint8_handler.invoke();
    }
    if (shouldInvokeFor(current, changed, _BV(1))) {
        _pcint9_handler.invoke();
    }
    if (shouldInvokeFor(current, changed, _BV(2))) {
        _pcint10_handler.invoke();
    }
    if (shouldInvokeFor(current, changed, _BV(3))) {
        _pcint11_handler.invoke();
    }
    if (shouldInvokeFor(current, changed, _BV(4))) {
        _pcint12_handler.invoke();
    }
    if (shouldInvokeFor(current, changed, _BV(5))) {
        _pcint13_handler.invoke();
    }

    lastPortC = current;
}
