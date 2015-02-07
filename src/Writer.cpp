#include "Writer.hpp"

Writer &Writer::operator << (const uint8_t b) {
    write(b);
    return *this;
}

Writer &Writer::operator << (const uint16_t i) {
    write(i >> 8);
    write(i);
    return *this;
}

Writer &Writer::operator << (const uint32_t i) {
    write(i >> 24);
    write(i >> 16);
    write(i >> 8);
    write(i);
    return *this;
}
