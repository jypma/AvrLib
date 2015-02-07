/*
 * Writer.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: jan
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <avr/common.h>

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

    Writer &operator << (const uint8_t b);
    Writer &operator << (const uint16_t i);
    Writer &operator << (const uint32_t i);

    /** Only use this for integer literals */
    inline Writer &operator << (int b) {
        return *this << uint16_t(b);
    }

    inline operator bool() const {
        return valid;
    }
};



#endif /* WRITER_HPP_ */
