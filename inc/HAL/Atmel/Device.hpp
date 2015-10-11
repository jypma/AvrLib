#ifndef HAL_DEVICE_HPP_
#define HAL_DEVICE_HPP_

#include <avr/io.h>

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#  include <HAL/Atmel/devices/ATmega328.hpp>
#else
#  error Unsupported chip
#endif


#endif /* HAL_DEVICE_HPP_ */
