
#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <avr/io.h>
#include "Streams/Writer.hpp"

extern uint16_t debugTimings[256];
extern uint8_t debugTimingsCount;

static uint16_t debugStartTime;

namespace Logging {

class TimingDisabled {
public:
    inline static void timeStart() {}
    inline static void timeEnd() {}
};

class TimingEnabled {
public:
    inline static void timeStart() {
        debugStartTime = TCNT1;
    }

    inline static void timeEnd() {
        uint16_t duration = TCNT1 - debugStartTime;

        if (debugTimingsCount < 254) {
            debugTimings[debugTimingsCount] = duration;
            debugTimingsCount++;
        }
    }
};

template <typename T>
class Log: public TimingDisabled {

};

}

#include "LoggingSettings.hpp"

template <typename pin_t>
void debugTimePrint(pin_t pin) {
    if (debugTimingsCount > 0) {
        for (uint8_t i = 0; i < debugTimingsCount; i++) {
          pin.out() << Streams::dec(debugTimings[i]) << " ";
        }
        pin.out() << Streams::endl;
    }
}

#endif /* DEBUG_HPP_ */
