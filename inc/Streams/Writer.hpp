/*
 * Writer.hpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jan
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_

#include "Reader.hpp"
#include "Streams/Format.hpp"
#include "EEPROM.hpp"
#include "gcc_type_traits.h"
#include <stdint.h>
#include "avr/eeprom.h"

namespace Streams {
constexpr const char endl[] = "\r\n";

template <typename fifo_t>
class BlockingWriteSemantics {
public:
    static inline bool canWrite(fifo_t &fifo, uint8_t size) {
        return true;
    }

    static inline void write(fifo_t &fifo, uint8_t value) {
        while (fifo.isFull()) ;

        fifo.uncheckedWrite(value);
    }

    static inline void start(fifo_t &fifo) {

    }

    static inline void end(fifo_t &fifo, bool valid) {

    }
};

template <typename fifo_t>
class NonblockingWriteSemantics {
public:
    static inline bool canWrite(fifo_t &fifo, uint8_t size) {
        return fifo.getSpace() >= size;
    }

    static inline void write(fifo_t &fifo, uint8_t value) {
        fifo.uncheckedWrite(value);
    }

    static inline void start(fifo_t &fifo) {
        fifo.writeStart();
    }

    static inline void end(fifo_t &fifo, bool valid) {
        if (valid) {
            fifo.writeEnd();
        } else {
            fifo.writeAbort();
        }
    }
};

template <typename fifo_t, typename sem = NonblockingWriteSemantics<fifo_t>>
class Writer {
    typedef Writer<fifo_t, sem> This;

    fifo_t *fifo;
    bool valid = true;

    template <typename T>
    inline void writeLiteral(const T value) {
        if (valid && sem::canWrite(*fifo, sizeof(T))) {
            for (uint8_t i = 0; i < sizeof(T); i++) {
                fifo->uncheckedWrite( ((uint8_t*)(&value))[i] );
            }
        } else {
            valid = false;
        }
    }

    void writeRange(const void * const ptr, const uint8_t length) {
        if (valid && sem::canWrite(*fifo, length)) {
            for (uint8_t i = 0; i < length; i++) {
                fifo->uncheckedWrite( ((uint8_t*)ptr)[i] );
            }
        } else {
            valid = false;
        }
    }

    static void writeByte(void *ctx, uint8_t value) {
        This *writer = (This*) ctx;
        writer->writeLiteral(value);
    }

public:
    inline Writer(fifo_t &_fifo): fifo(&_fifo) {
        sem::start(*fifo);
    }
    inline ~Writer() {
        sem::end(*fifo, valid);
    }

    inline void uncheckedWrite(uint8_t b) {
        fifo->uncheckedWrite(b);
    }

    inline operator bool() const {
        return valid;
    }

    template <uint8_t size>
    inline Writer &operator << (const char (&string)[size]) {
        writeRange(string, size - 1);
        return *this;
    }

    template <typename T, typename check = typename T::Proto>
    inline Writer & operator << (const T &t) {
        typedef typename T::Proto P;

        if (valid && sem::canWrite(*fifo, P::maximumUncheckedWriteSize)) {
            P::writeFields(t, *this);
        } else {
            valid = false;
        }

        return *this;
    }

    /** Writes a literal pointer (the actual address), LSB first (little endian) */
    template<typename T, typename check = typename std::enable_if<std::is_pointer<T>::value>::type>
    inline Writer &operator << (const T t) {
        writeLiteral(t);
        return *this;
    }

    /** Writes a single byte */
    inline Writer &operator << (const uint8_t value) {
        writeLiteral(value);
        return *this;
    }

    /** Writes a single uint16_t, LSB first (little endian) */
    inline Writer &operator << (const uint16_t value) {
        writeLiteral(value);
        return *this;
    }

    /** Writes a single uint32_t, LSB first (little endian) */
    inline Writer &operator << (const uint32_t value) {
        writeLiteral(value);
        return *this;
    }

    inline Writer &operator << (Decimal<uint8_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }
    inline Writer &operator << (Decimal<int8_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }
    inline Writer &operator << (Decimal<uint16_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }
    inline Writer &operator << (Decimal<int16_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }
    inline Writer &operator << (Decimal<uint32_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }
    inline Writer &operator << (Decimal<int32_t> v) {
        Format::format(&This::writeByte, this, v);
        return *this;
    }

    Writer &operator << (uint8_t EEPROM::*field) {
        uint8_t value = read(field);
        writeLiteral(value);
        return *this;
    }

    template <uint8_t size>
    Writer &operator << (char (EEPROM::*field)[size]) {
        uint8_t count = size;
        EEPROM *storage = (EEPROM *) 0;
        uint8_t *pos = (uint8_t *) (storage->*field);
        while (count > 0) {
            uint8_t value = eeprom_read_byte(pos);
            if (value == 0) {
                return *this;
            }
            writeLiteral(value);
            pos++;
            count--;
        }
        return *this;
    }

    Writer &operator << (Decimal<uint8_t EEPROM::*> field) {
        Format::format(&This::writeByte, this, dec(read(field.value)));
        return *this;
    }

    Writer &operator << (Decimal<uint16_t EEPROM::*> field) {
        Format::format(&This::writeByte, this, dec(read(field.value)));
        return *this;
    }

    /** Writes the remaining bytes of the reader into this writer. */
    template <typename reader_fifo_t>
    Writer &operator << (Reader<reader_fifo_t> &&in) {
        for (auto length = in.getReadAvailable(); length > 0; length--) {
            uint8_t value;
            in.uncheckedRead(value);
            writeLiteral(value);
        }
        return *this;
    }
};

}



#endif /* WRITER_HPP_ */
