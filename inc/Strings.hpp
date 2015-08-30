#ifndef STRINGS_H
#define STRINGS_H

#include "typestring.hh"

template <uint8_t length>
struct StringInProgmem {};

/**
 * Returns the given chars are a compile-time type-safe string, to be used as a value (e.g. method or operator argument)
 * The return type is a StringInProgmem<length> pointer, where the pointer address is the actual flash location of the string.
 */
#define F(chars) ( (StringInProgmem<typestring_is(chars)::size()> *) (typestring_is(chars)::data())  )

/**
 * Returns the given chars are a compile-time type-safe string, to be used as a type (e.g. template argument)
 */
#define STR(chars) typestring_is(chars)

#endif
