
#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include "Strings.hpp"

#include <avr/io.h>

#ifndef AVR
#include <stdarg.h>
#include <stdio.h>
#endif

constexpr uint8_t debugTimingCount = 32;

extern uint16_t debugTimings[debugTimingCount];
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

        if (debugTimingsCount < debugTimingCount) {
            debugTimings[debugTimingsCount] = duration;
            debugTimingsCount++;
        }
    }
};

struct MessagesDisabled {
    inline static void debug(const char *fmt, ...);
};

template <typename loggerName = STR("")>
struct MessagesEnabled {
#ifndef AVR
    inline static void debug(const char *fmt, ...) {
        printf("[%9s ] ", loggerName::data());
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
