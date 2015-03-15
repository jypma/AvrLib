#include "Reader.hpp"

Reader::~Reader()  {
    if (valid) {
        vtable->readCommit(delegate);
    } else {
        vtable->readRollback(delegate);
    }
}

void Reader::doRead(uint8_t &b) {
    read(b);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void Reader::doRead(uint16_t &i) {
    read(*((uint8_t*)(&i) + 1));
    read(*((uint8_t*)(&i) + 0));
}

void Reader::doRead(uint32_t &i) {
    read(*((uint8_t*)(&i) + 3));
    read(*((uint8_t*)(&i) + 2));
    read(*((uint8_t*)(&i) + 1));
    read(*((uint8_t*)(&i) + 0));
}
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif
