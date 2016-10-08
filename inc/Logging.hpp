
#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include "Strings.hpp"
#include "Fifo.hpp"
#include "Streams/Format.hpp"

#include <avr/io.h>

#ifndef AVR
#include <stdarg.h>
#include <stdio.h>
#include <mutex>
#endif

constexpr uint8_t debugTimingMax = 32;

extern volatile uint16_t debugTimings[debugTimingMax];
extern volatile uint8_t debugTimingsCount;

static uint16_t debugStartTime;

namespace Logging {

#ifndef AVR
namespace Impl {
extern std::mutex logging_mutex;
}
#endif

struct TimingDisabled {
    static constexpr bool isTimingEnabled() { return false; }
    inline static void timeStart() {}
    inline static void timeEnd() {}
};

// The overhead of enabling timing is 34 cycles per sample.
struct TimingEnabled {
    static constexpr bool isTimingEnabled() { return true; }

    __attribute__((always_inline)) inline static void timeStart() {
        debugStartTime = TCNT1;
    }

    __attribute__((always_inline)) inline static void timeEnd() {
        uint16_t duration = TCNT1 - debugStartTime;

        if (debugTimingsCount < debugTimingMax) {
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

extern void onFlush();

#define LOGGING_TO(var) \
	template <typename... types> void ::Logging::onMessage(types... args) { AtomicScope _; var.writeIfSpace(args...); } \
	void ::Logging::onFlush() { var.flush(); }

template <typename loggerName = STR("")>
struct MessagesEnabled {
    static constexpr bool isDebugEnabled() { return true; }
#ifndef AVR
    template <typename... types>
    inline static void debug(types... args) {
    	std::lock_guard<std::mutex> lock(Impl::logging_mutex);
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
    static void flush() {}
#else
    template <typename... types>
    inline static void debug(types... args) {
        onMessage(loggerName::instance(), F(": "), args..., ::Streams::endl);
    }

    static void flush() {
    	onFlush();
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
    constexpr uint8_t MAX = 15;
    if (debugTimingsCount > 0) {
    	uint8_t count = (debugTimingsCount >= MAX) ? MAX : debugTimingsCount;
		log::debug(Streams::Decimal(debugTimings, 0, count));
		{
			AtomicScope _;
			for (uint8_t i = 0; i < count; i++) {
				debugTimings[i] = debugTimings[i + count];
			}
			debugTimingsCount -= count;
		}
    }
}

}

#endif /* DEBUG_HPP_ */
