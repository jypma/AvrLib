#include "RealTimer.hpp"

uint8_t _watchdogCounter;

ISR(WDT_vect) {
    _watchdogCounter++;
}
