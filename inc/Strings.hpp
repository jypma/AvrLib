#ifndef STRINGS_H
#define STRINGS_H

#include "typestring.hh"

template <uint8_t... bytes>
using ByteString = irqus::typestring<bytes...>;

/**
 * Returns the given chars are a compile-time type-safe string, to be used as a value (e.g. method or operator argument)
 * The return type is a StringInProgmem<length> pointer, where the pointer address is the actual flash location of the string.
 *
 * For example, F("hello").
 *
 * F stands for Flash. (following the Arduino convention).
 */
#define F(chars) ( (StringInProgmem<typestring_is(chars)::size()> *) (typestring_is(chars)::data())  )

/**
 * Same as F, but takes individual bytes as arguments.
 *
 * For example, FB(0xDD,0,16).
 */
#define FB(...) ( (StringInProgmem<ByteString< __VA_ARGS__> ::size()> *) (ByteString< __VA_ARGS__ >::data())  )

/**
 * Returns the given chars are a compile-time type-safe string, to be used as a type (e.g. template argument),
 * e.g. MyTemplate<STR("hello")>
 */
#define STR(chars) typestring_is(chars)

/**
 * Returns the given bytes as a compile-time type-safe string, to be used as a type (e.g. template argument),
 * e.g. MyTemplate<STRB(1,2,3)>.
 */
#define STRB(...) ByteString<__VA_ARGS__>

#endif
