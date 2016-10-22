#pragma once

#include <stdint.h>
#include "attributes.hpp"

#ifndef AVR
#include <ostream>
extern uint8_t sfr_mem[256];
#endif

namespace HAL {

template<uintptr_t addr>
class Register16 {
    typedef Register16<addr> This;
    uint16_t value;
public:
#ifdef AVR
    static INLINE This &reg() { return *((This*) addr); }
#else
    static INLINE This &reg() { return *((This*) sfr_mem + addr); }
#endif
    static constexpr uintptr_t address = addr;

  INLINE uint16_t get() const volatile { return value; }
  INLINE void set(uint16_t v) volatile { value = v; }
  INLINE volatile uint16_t &val() volatile { return value; }
  INLINE  uint16_t &val() { return value; }
};

template <typename Reg>
class StaticRegister16 {
public:
    static INLINE Reg &reg() { return Reg::reg(); }
    INLINE static uint16_t get() { return reg().get(); }
    INLINE static void set(uint16_t v) { reg().set(v); }
  INLINE static uint16_t &val() { return reg().val(); }
};



}


