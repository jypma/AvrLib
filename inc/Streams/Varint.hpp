#pragma once

#include <stdint.h>

namespace Streams {
namespace Impl {
namespace Protobuf {

enum WireTypes: uint8_t {
    VARINT = 0,
    LENGTH_DELIMITED = 2
};

inline uint8_t
varint_size(uint32_t v)
{
    if (v < (1UL << 7)) {
        return 1;
    } else if (v < (1UL << 14)) {
        return 2;
    } else if (v < (1UL << 21)) {
        return 3;
    } else if (v < (1UL << 28)) {
        return 4;
    } else {
        return 5;
    }
}

inline uint8_t
varint_size(uint16_t v)
{
    if (v < (1UL << 7)) {
        return 1;
    } else if (v < (1UL << 14)) {
        return 2;
    } else {
        return 3;
    }
}

inline uint8_t
varint_size(uint8_t v)
{
    if (v < (1UL << 7)) {
        return 1;
    } else {
        return 2;
    }
}

inline uint32_t
zigzag(int32_t v)
{
    if (v < 0)
        return (-(uint32_t)v) * 2 - 1;
    else
        return (uint32_t)(v) * 2;
}

inline uint16_t
zigzag(int16_t v)
{
    if (v < 0)
        return (-(uint16_t)v) * 2 - 1;
    else
        return (uint16_t)(v) * 2;
}

inline uint8_t
zigzag(int8_t v)
{
    if (v < 0) {
        return (-(uint8_t)v) * 2 - 1;
    } else
        return (uint8_t)(v) * 2;
}

}
}
}

