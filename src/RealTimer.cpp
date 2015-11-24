#include "Time/RealTimer.hpp"

namespace Time {

uint8_t _watchdogCounter;

bool AbstractPeriodic::isNow(uint32_t currentTime, uint32_t delay) {
    AtomicScope _;

    if (waitForOverflow) {
        if (currentTime > 0xFFFFFFF) {
            return false;
        } else {
            waitForOverflow = false;
        }
    }
    if (currentTime >= next) {
        calculateNextCounts(next, delay);
        return true;
    } else {
        return false;
    }
}

void AbstractPeriodic::calculateNextCounts(uint32_t startTime, uint32_t delay) {
    next = startTime + delay;
    waitForOverflow = (next < startTime);
}

bool AbstractDeadline::isNow(uint32_t currentTime) {
    AtomicScope _;

    if (!elapsed) {
        if (waitForOverflow) {
            if (currentTime > 0xFFFFFFF) {
                return false;
            } else {
                waitForOverflow = false;
            }
        }
        if (currentTime >= next) {
            elapsed = true;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void AbstractDeadline::calculateNextCounts(uint32_t startTime, uint32_t delay) {
    next = startTime + delay;
    waitForOverflow = (next < startTime);
}


}

ISR(WDT_vect) {
    Time::_watchdogCounter++;
}
