#include "RS232.hpp"

// Start bit is always low
const uint8_t RS232::prefix[] = { 0b0 };

// Stop bit(s) are always high
const uint8_t RS232::postfix[] = { 0b11 };
