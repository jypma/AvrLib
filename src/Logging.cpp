#include "Logging.hpp"

uint16_t debugTimings[debugTimingCount];
uint8_t debugTimingsCount = 0;

#ifndef AVR
namespace Logging {
namespace Impl {
std::mutex logging_mutex;
}
}
#endif
