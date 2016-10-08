#include "Logging.hpp"

volatile uint16_t debugTimings[debugTimingMax];
volatile uint8_t debugTimingsCount = 0;

volatile uint16_t pls = 0;

#ifndef AVR
namespace Logging {
namespace Impl {
std::mutex logging_mutex;
}
}
#endif
