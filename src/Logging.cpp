#include "Logging.hpp"

uint16_t debugTimings[debugTimingCount];
uint8_t debugTimingsCount = 0;

namespace Logging {

void (*onMessage)(const char *msg) = nullptr;

}
