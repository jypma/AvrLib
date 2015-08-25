
#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <avr/io.h>

#ifndef AVR
#include <stdarg.h>
#include <stdio.h>
#endif

extern uint16_t debugTimings[256];
extern uint8_t debugTimingsCount;

static uint16_t debugStartTime;

namespace Logging {

struct TimingDisabled {
    inline static void timeStart() {}
    inline static void timeEnd() {}
};

struct TimingEnabled {
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

struct MessagesDisabled {
    inline static void debug(const char *fmt, ...);
};

struct MessagesEnabled {
#ifndef AVR
    inline static void debug(const char *fmt, ...) {
        va_list argp;
        va_start(argp, fmt);
        vprintf(fmt, argp);
        va_end(argp);
    }
#else
    inline static void debug(const char *fmt, ...) {
        // TODO implement on-chip debug message mechanism, pluggable? wifi?
    }
#endif
};

template <typename T>
struct Log: public TimingDisabled {

};

}

#include "LoggingSettings.hpp"

template <typename pin_t>
void debugTimePrint(pin_t pin) {
    /*
     * TODO route this to message logging instead
    if (debugTimingsCount > 0) {
        for (uint8_t i = 0; i < debugTimingsCount; i++) {
          pin.out() << Streams::dec(debugTimings[i]) << " ";
        }
        pin.out() << Streams::endl;
    }
    */
}

#endif /* DEBUG_HPP_ */
