#ifndef STREAMS_WRITINGLITTLEENDIAN_HPP_
#define STREAMS_WRITINGLITTLEENDIAN_HPP_

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif


#include "EEPROM.hpp"
#include <stdint.h>

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t, typename T>
inline bool __attribute__((optimize("unroll-loops"))) writeLiteralByteValue(fifo_t &fifo, const T value) {
    //if (sem::canWrite(fifo, sizeof(T))) {
        for (uint8_t i = 0; i < sizeof(T); i++) {
            sem::write(fifo, ((uint8_t*)(&value))[i] );
        }
        return true;
    //} else {
    //    return false;
    //}
}

template <typename sem, uint8_t count, typename fifo_t>
inline bool __attribute__((optimize("unroll-loops"))) writeLiteralBytes(fifo_t &fifo, const uint8_t *src) {
    //if (sem::canWrite(fifo, count)) {
        for (uint8_t i = 0; i < count; i++) {
            sem::write(fifo, src[i] );
        }
        return true;
    //} else {
    //    return false;
    //}
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const bool value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t, typename enum_t>
bool write1unchecked(fifo_t &fifo, enum_t v, typename enum_t::type *test = nullptr) {
    return writeLiteralByteValue<sem>(fifo, v.underlying());
}

template <typename sem, typename fifo_t, uint8_t count>
bool write1unchecked(fifo_t &fifo, const uint8_t (*value) [count]) {
    return writeLiteralBytes<sem, count>(fifo, *value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const uint8_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const int8_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const uint16_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const int16_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const uint32_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const int32_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const uint64_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const int64_t value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, const char value) {
    return writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, uint8_t EEPROM::*field) {
    return writeLiteralByteValue<sem>(fifo, read(field));
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, uint16_t EEPROM::*field) {
    return writeLiteralByteValue<sem>(fifo, read(field));
}

template <typename sem, typename fifo_t>
bool write1unchecked(fifo_t &fifo, uint32_t EEPROM::*field) {
    return writeLiteralByteValue<sem>(fifo, read(field));
}

}
}



#endif /* STREAMS_WRITINGLITTLEENDIAN_HPP_ */
