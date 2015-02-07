/*
 * Reader.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: jan
 */

#ifndef READER_HPP_
#define READER_HPP_

#include <avr/common.h>

class Reader {
public:
    struct VTable {
        void (*readStart)(void *);
        void (*readEnd)(void *);
        bool (*read)(void *, uint8_t &);
        uint8_t (*getRemaining)(void*);
    };
private:
    const VTable * const vtable;
    void * const delegate;
    bool valid;

    void read(uint8_t &b) {
        if (valid) {
            valid = vtable->read(delegate, b);
        }
    }
    Reader(): vtable(nullptr), delegate(nullptr), valid(false) {}
public:
    inline Reader(const VTable *_vtable, void *_delegate): vtable(_vtable), delegate(_delegate), valid(true) {
        vtable->readStart(delegate);
    }
    inline ~Reader() {
        vtable->readEnd(delegate);
    }

    Reader &operator >> (uint8_t &b);
    Reader &operator >> (uint16_t &i);
    Reader &operator >> (uint32_t &i);

    inline operator bool() const {
        return valid;
    }

    inline uint8_t getRemaining() const {
        return vtable->getRemaining(delegate);
    }
};



#endif /* READER_HPP_ */
