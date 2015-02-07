#include "Reader.hpp"

Reader &Reader::operator >> (uint8_t &b) {
    read(b);
    return *this;
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
Reader &Reader::operator >> (uint16_t &i) {
    read(*((uint8_t*)(&i) + 1));
    read(*((uint8_t*)(&i) + 0));
    return *this;
}

Reader &Reader::operator >> (uint32_t &i) {
    read(*((uint8_t*)(&i) + 3));
    read(*((uint8_t*)(&i) + 2));
    read(*((uint8_t*)(&i) + 1));
    read(*((uint8_t*)(&i) + 0));
    return *this;
}
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif
