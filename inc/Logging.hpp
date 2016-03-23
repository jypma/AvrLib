
#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include "Strings.hpp"
#include "Fifo.hpp"
#include "Streams/Format.hpp"

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
    template <typename... types>
    inline static void debug(types... args) {}

    static constexpr bool isDebugEnabled() { return false; }
};

template <typename... types>
extern void onMessage(types... args);

#define LOGGING_TO(var) template <typename... types> void ::Logging::onMessage(types... args) { var.writeIfSpace(args...); }

template <typename loggerName = STR("")>
struct MessagesEnabled {
    static constexpr bool isDebugEnabled() { return true; }
#ifndef AVR
    template <typename... types>
    inline static void debug(types... args) {
        printf("[%9s ] ", loggerName::data());
        Fifo<250> out;
        out.write(args...);
        while (out.hasContent()) {
            uint8_t ch;
            out.read(&ch);
            printf("%c", ch);
        }
        printf("\n");
    }
#else
    template <typename... types>
    inline static void debug(types... args) {
        onMessage(loggerName::instance(), F(": "), args..., ::Streams::endl);
    }
#endif
};

template <typename T>
struct Log: public TimingDisabled, public MessagesDisabled{

};

}

#include "LoggingSettings.hpp"

namespace Logging {

inline void printTimings() {
    typedef Logging::Log<Loggers::Timing> log;
    //static uint8_t lastCount = 0;
    //if (debugTimingCount != lastCount) {
        log::debug(Streams::Decimal(debugTimings, 0, debugTimingsCount));
    //    lastCount = debugTimingCount;
    //}
}

}

#endif /* DEBUG_HPP_ */
