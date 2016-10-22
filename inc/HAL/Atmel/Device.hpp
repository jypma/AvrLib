#pragma once

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#  include "devices/ATmega328p.hpp"
#else
#  error Unsupported chip
#endif
