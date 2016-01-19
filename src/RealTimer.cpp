#include "Time/RealTimer.hpp"

namespace Time {

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

uint32_t AbstractDeadline::getTimeLeft(uint32_t currentTime) const {
    AtomicScope _;

    if (!elapsed) {
        if (waitForOverflow) {
            return next - currentTime;
        }
        if (currentTime >= next) {
            // has elapsed, but isNow hasn't been called yet.
            return 0;
        } else {
            return next - currentTime;
        }
    } else {
        return 0xFFFFFFFF;
    }
}

void AbstractDeadline::calculateNextCounts(uint32_t startTime, uint32_t delay) {
    next = startTime + delay;
    waitForOverflow = (next < startTime);
}


}

