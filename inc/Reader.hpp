/*
 * Reader.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: jan
 */

#ifndef READER_HPP_
#define READER_HPP_

#include <avr/common.h>
#include "gcc_type_traits.h"

class Reader {
public:
    struct VTable {
        void (*readStart)(void *);
        void (*readCommit)(void *);
        void (*readRollback)(void *);
        bool (*read)(void *, uint8_t &);
        uint8_t (*getRemaining)(void*);
        bool (*isReading)(void*);

        VTable(
            void (*_readStart)(void *),
            void (*_readCommit)(void *),
            void (*_readRollback)(void *),
            bool (*_read)(void *, uint8_t &),
            uint8_t (*_getRemaining)(void*),
            bool (*_isReading)(void*)
        ):
            readStart(_readStart),
            readCommit(_readCommit),
            readRollback(_readRollback),
            read(_read),
            getRemaining(_getRemaining),
            isReading(_isReading)
        {}
    };
private:
    const VTable * const vtable;
    void * const delegate;
    bool valid;
    bool wasReading;

    void read(uint8_t &b) {
        if (valid) {
            valid = vtable->read(delegate, b);
        }
    }

    template<typename T>
    inline void doRead(T& t, typename std::enable_if<std::is_enum<T>::value>::type* = 0)
    {
        *this >> *((typename std::underlying_type<T>::type *) &t);
    }

    template<typename T>
    inline void doRead(T& t, typename std::enable_if<std::is_class<T>::value>::type* = 0)
    {
        T::read(*this, t);
    }

    template<typename T>
    void doRead(T& t, typename std::enable_if<std::is_pointer<T>::value>::type* = 0)
    {
        for (uint8_t i = 0; i < sizeof(T); i++) {
            read(*((uint8_t*)(&t) + i));
        }
    }

    void doRead(uint8_t &b);
    void doRead(uint16_t &i);
    void doRead(uint32_t &i);
    /** Reads a single byte. Sets the boolean to false if it's 0, or true otherwise. */
    void doRead(bool &b);

    //Reader(): vtable(nullptr), delegate(nullptr), valid(false), wasReading(false) {}
public:
    inline Reader(const VTable *_vtable, void *_delegate): vtable(_vtable), delegate(_delegate), valid(true) {
        wasReading = vtable->isReading(delegate);
        if (!wasReading) {
            vtable->readStart(delegate);
        }
    }
    ~Reader();

    inline operator bool() const {
        return valid;
    }

    inline uint8_t getRemaining() const {
        return vtable->getRemaining(delegate);
    }

    template<typename T>
    inline Reader &operator >> (T &t) {
        doRead(t);
        return *this;
    }

};

#endif /* READER_HPP_ */
