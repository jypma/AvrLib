#pragma once

#include <stdint.h>
#include "attributes.hpp"
#include "Bit.hpp"

#ifndef AVR
#include <ostream>
extern uint8_t sfr_mem[256];
#endif

namespace HAL {

template <typename Reg, uint8_t bit> struct RegisterBit {};
template <typename Reg> struct RegisterBit<Reg, 0> { using type = typename Reg::Bit0; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 1> { using type = typename Reg::Bit1; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 2> { using type = typename Reg::Bit2; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 3> { using type = typename Reg::Bit3; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 4> { using type = typename Reg::Bit4; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 5> { using type = typename Reg::Bit5; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 6> { using type = typename Reg::Bit6; static constexpr type value = {}; };
template <typename Reg> struct RegisterBit<Reg, 7> { using type = typename Reg::Bit7; static constexpr type value = {}; };

#ifndef AVR
    /** Internal API for unit tests. Do not call directly. */
    void Register8_onChange(volatile void *self);
#endif

/**
 * An 8-bit register with each bit known to be reserved, read-only or read-write.
 * No implicit conversions to uint8_t, since that would make it too easy to lose type-safety and/or
 * make expressions ambiguous.
 */
template <
  uintptr_t addr,
  template<typename, uint8_t> class _bit0,
  template<typename, uint8_t> class _bit1,
  template<typename, uint8_t> class _bit2,
  template<typename, uint8_t> class _bit3,
  template<typename, uint8_t> class _bit4,
  template<typename, uint8_t> class _bit5,
  template<typename, uint8_t> class _bit6,
  template<typename, uint8_t> class _bit7
>
class Register8 {
private:
	using This = Register8<addr, _bit0, _bit1, _bit2, _bit3, _bit4, _bit5, _bit6, _bit7>;

public:
	uint8_t value; // TODO fix permissions wrt Bit.hpp, probably by making reg() point to StaticRegister8 and only invoking statics.

#ifdef AVR
    static INLINE volatile This &reg() { return *((volatile This*) addr); }
#else
    static INLINE volatile This &reg() { return *((volatile This*) sfr_mem + addr); }
#endif
	INLINE Register8() { value = reg().value; }
	INLINE Register8(const This &that) { value = that.value; }

	static constexpr uintptr_t address = addr;

	/** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be read. */
	INLINE uint8_t get() const volatile { return value; }
    /** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be written to. */
	INLINE void set(uint8_t v) volatile { this->value = v; }
    /** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be read. */
    INLINE uint8_t &val() { return value; }
    /** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be read. */
    INLINE volatile uint8_t &val() volatile { return value; }


    constexpr Register8(const volatile This &v): value(v.value) {}

    explicit constexpr Register8(uint8_t v): value(v) {}

    template <typename Mask>
	constexpr Register8(const HAL::Bit<This,Mask> t): value(Mask::bitMask) {}

	static constexpr uint8_t size = 1;

	template <uint8_t b>
	using Bit = typename RegisterBit<This, b>::type;

    template <uint8_t b>
	static constexpr Bit<b> bit() { return {}; }

	using Bit0 = _bit0<This, 0>;
	using Bit1 = _bit1<This, 1>;
	using Bit2 = _bit2<This, 2>;
	using Bit3 = _bit3<This, 3>;
	using Bit4 = _bit4<This, 4>;
	using Bit5 = _bit5<This, 5>;
	using Bit6 = _bit6<This, 6>;
	using Bit7 = _bit7<This, 7>;

    INLINE void operator=(const This &t) volatile {
        value = t.value;
#ifndef AVR
        Register8_onChange(this);
#endif
    }

	template <typename Mask>
	INLINE void operator=(const HAL::Bit<This,Mask> bit) volatile {
        // TODO don't assign reserved bits here
	    value = Mask::bitMask;
#ifndef AVR
	    Register8_onChange(this);
#endif
	}

    /**
     * Sets the given bitmask, e.g. CS |= CS00 | CS01;
     */
	template <typename Mask, typename check = typename std::enable_if<Mask::applyMask == 0xFF>::type>
	                                                             //  must not have used & operator
	INLINE void operator|=(HAL::Bit<This,Mask>) volatile {
	    value |= Mask::bitMask;
#ifndef AVR
	    Register8_onChange(this);
#endif
	}

	/**
	 * Clears the inverse of the given bitmask, e.g. CS &= ~(CS00 | CS01);
	 */
    template <typename Mask, typename check = typename std::enable_if<Mask::bitMask == 0>::type>
                                                                  //  must not have used | operator
	INLINE void operator&=(HAL::Bit<This,Mask>) volatile {
	    value &= Mask::applyMask;
#ifndef AVR
	    Register8_onChange(this);
#endif
	}

    /**
     * Sets the bits in the expression, and clears the inverted bits in the expression.
     * e.g. CS.apply(CS00 | ~CS01 | CS02) will set CS00 and CS02, clear CS01, and leave other bits alone.
     */
	template <typename Mask>
	INLINE void apply(HAL::Bit<This,Mask>) volatile {
        value = (value & Mask::applyMask) | Mask::bitMask;
#ifndef AVR
        Register8_onChange(this);
#endif
	}

	/**
	 * Returns whether exactly the bits given in the mask are set, and all other bits are zero.
	 */
	template <typename Mask>
	INLINE bool operator== (HAL::Bit<This,Mask>) const volatile {
	    return (value == Mask::bitMask);
	}

    /**
     * Returns whether exactly the bits given in the mask are set, ignoring other bits.
     */
    template <typename Mask>
    INLINE bool matches (HAL::Bit<This,Mask>) const volatile {
        return (value & Mask::applyMask) == Mask::bitMask;
    }

/*
	template <typename T, typename check = typename std::enable_if<std::is_same<typename T::Reg, This>::value>::type>
	INLINE This operator &(T t) const volatile { return value & (T::applyMask & T::bitMask); }

    template <typename T, typename check = typename std::enable_if<std::is_same<typename T::Reg, This>::value>::type>
    INLINE This operator |(T t) const volatile { return value | (T::applyMask & T::bitMask); }
*/
};

template <typename Reg>
class StaticRegister8 {
private:
    using This = StaticRegister8<Reg>;
    static INLINE volatile Reg &reg() { return Reg::reg(); }

public:
    /** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be read. */
    static INLINE uint8_t get() { return reg().get(); }
    /** Direct access to the uint8_t content of the register. TODO move to only be on registers where all 8 bits can be written to. */
    static INLINE void set(uint8_t v) { reg().set(v); }
    static INLINE volatile uint8_t &val() { return reg().val(); }

    //StaticRegister8(const This &that) = delete;
    This& operator=(This const&) = delete;

    constexpr INLINE operator Reg() volatile const { return reg(); }

    static constexpr uint8_t size = 1;

    template <uint8_t b>
    using Bit = typename RegisterBit<Reg, b>::type;

    template <uint8_t b>
    static constexpr Bit<b> bit() { return {}; }

    using Bit0 = typename Reg::Bit0;
    using Bit1 = typename Reg::Bit1;
    using Bit2 = typename Reg::Bit2;
    using Bit3 = typename Reg::Bit3;
    using Bit4 = typename Reg::Bit4;
    using Bit5 = typename Reg::Bit5;
    using Bit6 = typename Reg::Bit6;
    using Bit7 = typename Reg::Bit7;

    INLINE void operator=(Reg t) volatile const { reg() = t; }

    template <typename Mask>
    INLINE void operator=(const HAL::Bit<Reg,Mask> bit) volatile const { reg() = bit; }

    template <typename Mask>
    INLINE void operator|=(const HAL::Bit<Reg,Mask> bit) volatile const { reg() |= bit; }

    template <typename Mask>
    INLINE void operator&=(const HAL::Bit<Reg,Mask> bit) volatile const { reg() &= bit; }

    template <typename Mask>
    INLINE void apply(const HAL::Bit<Reg,Mask> bit) volatile const { reg().apply(bit); }

    template <typename Mask>
    INLINE bool operator== (HAL::Bit<Reg,Mask> bit) const volatile { return reg() == bit; }

    template <typename Mask>
    INLINE bool matches (HAL::Bit<Reg,Mask> bit) const volatile { return reg().matches(bit); }

    template <typename T>
    INLINE Reg operator &(T t) const volatile { return reg() & t; }

    template <typename T>
    INLINE Reg operator |(T t) const volatile { return reg() | t; }
};

#ifndef AVR
template <
  uintptr_t addr,
  template<typename, uint8_t> class _bit0,
  template<typename, uint8_t> class _bit1,
  template<typename, uint8_t> class _bit2,
  template<typename, uint8_t> class _bit3,
  template<typename, uint8_t> class _bit4,
  template<typename, uint8_t> class _bit5,
  template<typename, uint8_t> class _bit6,
  template<typename, uint8_t> class _bit7
>
inline ::std::ostream& operator<<(::std::ostream& os, const volatile Register8<addr,_bit0,_bit1,_bit2,_bit3,_bit4,_bit5,_bit6,_bit7>& that) {
    return os << int(that.val());
}

template <typename Reg>
inline ::std::ostream& operator<<(::std::ostream& os, const volatile StaticRegister8<Reg> &that) {
    return os << int(that.val());
}
#endif

}
