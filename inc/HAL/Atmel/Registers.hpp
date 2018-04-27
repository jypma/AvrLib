#pragma once

#include <stdint.h>

#ifndef AVR
    extern uint8_t sfr_mem[256];
#endif

// these can't be constexpr because C++ decided pointer <-> int can't be constexpr.
#ifdef AVR
#define sfrStartAddress (0)
#else
#define sfrStartAddress ((uintptr_t) &sfr_mem)
#endif

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#  include "registers/ATMega328p_io.hpp"
#else
#  error Unsupported chip
#endif

namespace HAL {
namespace Atmel {
namespace Registers {

using SREG_t = Register8<
        0x3F + 0x20,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit,
        ReadWriteBit>;
constexpr StaticRegister8<SREG_t> SREG = {};
constexpr SREG_t::Bit0 SREG_C = {};
constexpr SREG_t::Bit1 SREG_Z = {};
constexpr SREG_t::Bit2 SREG_N = {};
constexpr SREG_t::Bit3 SREG_V = {};
constexpr SREG_t::Bit4 SREG_S = {};
constexpr SREG_t::Bit5 SREG_H = {};
constexpr SREG_t::Bit6 SREG_T = {};
constexpr SREG_t::Bit7 SREG_I = {};

}
}
}

