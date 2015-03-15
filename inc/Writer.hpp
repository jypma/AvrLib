/*
 * Writer.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: jan
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <stdint.h>
#include "gcc_type_traits.h"

template <typename T>
struct Decimal {
    T value;
};

template <typename T>
inline Decimal<T> dec(T v) {
    return Decimal<T> { v };
}

class Writer {
public:
    struct VTable {
        void (*writeStart)(void *);
        void (*writeCommit)(void *);
        void (*writeRollback)(void *);
        bool (*write)(void *, uint8_t);
    };
private:
    const VTable * const vtable;
    void * const delegate;
    bool valid;

    void write(const uint8_t b);

    template<typename T>
    inline void doWrite(const T& t, typename std::enable_if<std::is_enum<T>::value>::type* = 0)
    {
        *this << static_cast<typename std::underlying_type<T>::type>(t);
    }

    template<typename T>
    inline void doWrite(const T& t, typename std::enable_if<std::is_class<T>::value>::type* = 0)
    {
       T::write(*this, t);
    }

    void doWrite(const char *string);

    Writer(): vtable(nullptr), delegate(nullptr), valid(false) {}
public:
    inline Writer(const VTable *_vtable, void *_delegate): vtable(_vtable), delegate(_delegate), valid(true) {
        vtable->writeStart(delegate);
    }
    ~Writer();

    /** Writes a single byte */
    Writer &operator << (const uint8_t b);
    /** Writes a 16-bit word as MSB first */
    Writer &operator << (const uint16_t i);
    /** Writes a 32-bit word as MSB first */
    Writer &operator << (const uint32_t i);

    /** Writes a 16-bit word as MSB first. Only use this for integer literals */
    inline Writer &operator << (int b) {
        return *this << uint16_t(b);
    }

    /** Writes the given value as decimal */
    Writer &operator << (Decimal<uint8_t> v);
    /** Writes the given value as decimal */
    Writer &operator << (Decimal<int8_t> v);
    /** Writes the given value as decimal */
    Writer &operator << (Decimal<uint16_t> v);
    /** Writes the given value as decimal */
    Writer &operator << (Decimal<int16_t> v);
    /** Writes the given value as decimal */
    Writer &operator << (Decimal<uint32_t> v);
    /** Writes the given value as decimal */
    Writer &operator << (Decimal<int32_t> v);

    inline operator bool() const {
        return valid;
    }

    template<typename T>
    inline Writer &operator << (const T &t) {
        doWrite(t);
        return *this;
    }
};

#endif /* WRITER_HPP_ */
