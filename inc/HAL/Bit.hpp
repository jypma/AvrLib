#pragma once

#include <stdint.h>
#include "gcc_type_traits.h"
#include "attributes.hpp"

namespace HAL {

template<typename l, typename r>
struct Or;
template<typename l, typename r>
struct And;
template<typename t>
struct Not;

template <typename Reg>
class StaticRegister8;

template<typename _Reg, typename _Mask>
class Bit {
public:
	using Reg = _Reg;
	using Mask = _Mask;
	static constexpr auto bitMask = Mask::bitMask;
	static constexpr auto applyMask = Mask::applyMask;

	template <typename r, typename check = typename std::enable_if<std::is_same<typename r::Reg, Reg>::value>::type>
	Bit<_Reg, Or<Mask, typename r::Mask>> constexpr operator| (r expr) const volatile { return {}; };

	template <typename r, typename check = typename std::enable_if<std::is_same<typename r::Reg, Reg>::value>::type>
	Bit<_Reg, And<Mask, typename r::Mask>> constexpr operator& (r expr) const volatile { return {}; };

	Bit<_Reg, Not<Mask>> constexpr operator~() const volatile { return {}; }

	INLINE bool operator== (const volatile Reg &reg) const volatile {
	    return reg.val() == bitMask;
	}

    INLINE bool operator== (const volatile StaticRegister8<Reg> &reg) const volatile {
        return reg.val() == bitMask;
    }
};

template<typename _Reg, typename This>
INLINE void operator |= (uint8_t &i, const Bit<_Reg,This> bits) {
    i |= This::bitMask;
}

template<typename _Reg, typename This>
INLINE void operator &= (uint8_t &i, const Bit<_Reg,This> bits) {
    i &= This::bitMask;
}

template<typename _Reg, typename This>
INLINE uint8_t operator | (const uint8_t i, const Bit<_Reg,This> bits) {
    return i | This::bitMask;
}

template<typename _Reg, typename This>
INLINE uint8_t operator & (const uint8_t i, const Bit<_Reg,This> bits) {
    return i & This::bitMask;
}

template<typename t>
class Not {
public:
	static constexpr uint8_t bitMask = (uint8_t) (~t::bitMask);
	static constexpr uint8_t applyMask = t::applyMask;
};

template<typename l, typename r>
class Or {
public:
	static constexpr uint8_t bitMask =  l::bitMask | (r::bitMask & r::applyMask);
	static constexpr uint8_t applyMask =  l::applyMask | r::applyMask;
};

template<typename l, typename r>
class And {
public:
	static constexpr uint8_t bitMask =  l::bitMask & r::bitMask;
	static constexpr uint8_t applyMask =  l::applyMask | r::applyMask;
};

template <uint8_t _idx>
struct MaskBit {
	static constexpr uint8_t idx = _idx;
    static constexpr uint8_t bitMask = 1 << idx;
    static constexpr uint8_t applyMask = bitMask;
};

/**
 * @param _mmptr_t MemoryMappedReference instantiation
 */
template<typename _Reg, uint8_t idx>
class ReservedBit: public Bit<_Reg, MaskBit<idx>>
{
public:
	using Reg = _Reg;
	static_assert(idx >= 0 && idx <= 7, "Index out of range! (0 <= idx <= 7)");
	static_assert(Reg::size == 1, "Only 8-bit registers are supported!");
};

template<class Reg, uint8_t idx>
class ReadOnlyBit : public ReservedBit<Reg, idx>
{
public:
	using Mask = MaskBit<idx>;

	/// Check if the bit is set.
	static bool INLINE isSet() {
	    return Reg::reg().val() & Mask::bitMask;
	}
	/// Check if the bit is cleared.
	static bool INLINE isCleared() { return !isSet(); }

	/// Returns 0 or 1 depending on whether the bit is cleared or set.
	static uint8_t INLINE getValue() { return (Reg::reg().val() & Mask::bitMask) << idx; }
};

template<class Reg, uint8_t idx>
class StatusBitClearBy0 : public ReadOnlyBit<Reg, idx>
{
public:
    using Mask = MaskBit<idx>;
	/// Clear the bit by writing a 0.
	static void INLINE clear()
	{
	    Reg::reg().value &= ~Mask::bitMask;
#ifndef AVR
	    Register8_onChange(&Reg::reg());
#endif
	}
};

template<class Reg, uint8_t idx>
class StatusBitClearBy1 : public ReadOnlyBit<Reg, idx>
{
public:
    using Mask = MaskBit<idx>;
	/// Clear the bit by writing a 1.
	static void INLINE clear()
	{
	    Reg::reg().value |= Mask::bitMask;
#ifndef AVR
	    Register8_onChange(&Reg::reg());
#endif
	}
};

template<class Reg, uint8_t idx>
class ReadWriteBit : public StatusBitClearBy0<Reg, idx>
{
public:
    using Mask = MaskBit<idx>;

	/// Set the bit.
	static void INLINE set()
	{
	    Reg::reg().value |= Mask::bitMask;
#ifndef AVR
	    Register8_onChange(&Reg::reg());
#endif
	}

	/// Set the bit to [high].
	static void INLINE apply(bool high) {
		if (high) {
		    Reg::reg().value |= Mask::bitMask;
		} else {
		    Reg::reg().value &= ~Mask::bitMask;
		}
#ifndef AVR
		Register8_onChange(&Reg::reg());
#endif
	}
};

}

