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
    } else if (next > 0xC0000000 && currentTime < 0x20000000) {
    	// we've missed an isNow() on our actual time, and now the timer has overflowed
        calculateNextCounts(currentTime, delay);
        return true;
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

uint32_t AbstractPeriodic::getTimeLeft(uint32_t currentTime) const {
    AtomicScope _;

	if (waitForOverflow) {
		if (currentTime < 0xFFFFFFF) {
			// has overflowed, but isNow hasn't been called yet
			return currentTime >= next ? 0 : next - currentTime;
		} else {
			return next - currentTime;
		}
	} else if (next > 0xC0000000 && currentTime < 0x20000000) {
		// we've missed an isNow() on our actual time, and now the timer has overflowed
		return 0;
	}
	if (currentTime >= next) {
		// has elapsed, but isNow hasn't been called yet.
		return 0;
	} else {
		return next - currentTime;
	}
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
        } else if (next > 0xC0000000 && currentTime < 0x20000000) {
        	// we've missed an isNow() on our actual time, and now the timer has overflowed
            elapsed = true;
            return true;
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
        	if (currentTime < 0xFFFFFFF) {
        		// has overflowed, but isNow hasn't been called yet
        		return currentTime >= next ? 0 : next - currentTime;
        	} else {
        		return next - currentTime;
        	}
        } else if (next > 0xC0000000 && currentTime < 0x20000000) {
        	// we've missed an isNow() on our actual time, and now the timer has overflowed
            return 0;
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

void AbstractDeadline::calculateNext(uint32_t startTime, uint32_t delay) {
    next = startTime + delay;
    waitForOverflow = (next < startTime);
}


}

