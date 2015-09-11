#include "Time/RealTimer.hpp"

namespace Time {

uint8_t _watchdogCounter;

}

ISR(WDT_vect) {
    Time::_watchdogCounter++;
}
