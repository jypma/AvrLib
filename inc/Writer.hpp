/*
 * Writer.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: jan
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <stdint.h>

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
        void (*writeEnd)(void *);
        bool (*write)(void *, uint8_t);
    };
private:
    const VTable * const vtable;
    void * const delegate;
    bool valid;

    void write(const uint8_t b) {
        if (valid) {
            valid = vtable->write(delegate, b);
        }
    }
    Writer(): vtable(nullptr), delegate(nullptr), valid(false) {}
public:
    inline Writer(const VTable *_vtable, void *_delegate): vtable(_vtable), delegate(_delegate), valid(true) {
        vtable->writeStart(delegate);
    }
    inline ~Writer() {
        vtable->writeEnd(delegate);
    }

    /** Writes a single byte */
    Writer &operator << (const uint8_t b);
    /** Writes a 16-bit word as MSB first */
    Writer &operator << (const uint16_t i);
    /** Writes a 32-bit word as MSB first */
    Writer &operator << (const uint32_t i);
    /** Writes the bytes in the given null-terminated string */
    Writer &operator << (const char *string);

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

    inline operator bool() const {
        return valid;
    }
};



#endif /* WRITER_HPP_ */
