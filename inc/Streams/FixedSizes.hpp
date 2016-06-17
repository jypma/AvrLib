#ifndef STREAMS_FIXEDSIZES_HPP_
#define STREAMS_FIXEDSIZES_HPP_

#include <stdint.h>
#include "gcc_type_traits.h"
#include "TypeTraits.hpp"

namespace Streams {
namespace Impl {

template <typename T, typename check = void> struct StreamedSizeReading {
    static constexpr bool fixed = false;
};

template <typename T, typename check = void> struct StreamedSizeWriting {
    static constexpr bool fixed = false;
};

template <uint8_t s>
struct FixedSize {
    static constexpr bool fixed = true;
    static constexpr uint8_t size = s;
};

template<> struct StreamedSizeReading<uint8_t*>: public FixedSize<1> {};
template<> struct StreamedSizeWriting<uint8_t>: public FixedSize<1> {};
template<> struct StreamedSizeReading<int8_t*>: public FixedSize<1> {};
template<> struct StreamedSizeWriting<int8_t>: public FixedSize<1> {};
template<> struct StreamedSizeReading<uint16_t*>: public FixedSize<2> {};
template<> struct StreamedSizeWriting<uint16_t>: public FixedSize<2> {};
template<> struct StreamedSizeReading<int16_t*>: public FixedSize<2> {};
template<> struct StreamedSizeWriting<int16_t>: public FixedSize<2> {};
template<> struct StreamedSizeReading<uint32_t*>: public FixedSize<4> {};
template<> struct StreamedSizeWriting<uint32_t>: public FixedSize<4> {};
template<> struct StreamedSizeReading<int32_t*>: public FixedSize<4> {};
template<> struct StreamedSizeWriting<int32_t>: public FixedSize<4> {};
template<> struct StreamedSizeReading<uint64_t*>: public FixedSize<8> {};
template<> struct StreamedSizeWriting<uint64_t>: public FixedSize<8> {};
template<> struct StreamedSizeReading<int64_t*>: public FixedSize<8> {};
template<> struct StreamedSizeWriting<int64_t>: public FixedSize<8> {};

template<> struct StreamedSizeReading<char*>: public FixedSize<sizeof(char)> {};
template<> struct StreamedSizeWriting<char>: public FixedSize<sizeof(char)> {};
template<> struct StreamedSizeReading<char>: public FixedSize<sizeof(char)> {};

template<> struct StreamedSizeReading<bool*>: public FixedSize<1> {};
template<> struct StreamedSizeWriting<bool>: public FixedSize<1> {};

template<uint8_t count> struct StreamedSizeReading<uint8_t (*) [count]>: public FixedSize<count> {};
template<uint8_t count> struct StreamedSizeWriting<uint8_t (*) [count]>: public FixedSize<count> {};
template<uint8_t count> struct StreamedSizeWriting<const uint8_t (*) [count]>: public FixedSize<count> {};

template <typename enum_t>
struct StreamedSizeReading<enum_t, typename exists<typename std::remove_pointer<enum_t>::type::type>::type>:
    public FixedSize<sizeof(typename std::remove_pointer<enum_t>::type::type)> {};
template <typename enum_t>
struct StreamedSizeWriting<enum_t, typename exists<typename enum_t::type>::type>:
    public FixedSize<sizeof(typename enum_t::type)> {};

// reading EEPROM hasn't been defined+implemented yet. Would that read to eeprom, or just verify that read bytes match eeprom?
template<> struct StreamedSizeWriting<uint8_t EEPROM::*>: public FixedSize<1> {};
template<> struct StreamedSizeWriting<uint16_t EEPROM::*>: public FixedSize<2> {};
template<> struct StreamedSizeWriting<uint32_t EEPROM::*>: public FixedSize<4> {};

template <uint8_t length>
struct StreamedSizeWriting<StringInProgmem<length>*>: public FixedSize<length> {};

//we don't enable fixed-size reading(checking) of strings, since we need partial matches during scanning
//template <uint8_t length>
//struct StreamedSizeReading<StringInProgmem<length>*>: public FixedSize<length> {};

}

template <typename... types>
struct StreamedSize;

template<>
struct StreamedSize<> {
    constexpr static uint8_t fixedSizeReading = 0;
    constexpr static uint8_t fixedSizeWriting = 0;
};

template <typename head, typename... tail>
struct StreamedSize<head, tail...> {
    constexpr static uint8_t fixedSizeReading = Impl::StreamedSizeReading<head>::size + StreamedSize<tail...>::fixedSizeReading;
    constexpr static uint8_t fixedSizeWriting = Impl::StreamedSizeWriting<head>::size + StreamedSize<tail...>::fixedSizeWriting;
};

}



#endif /* STREAMS_FIXEDSIZES_HPP_ */
