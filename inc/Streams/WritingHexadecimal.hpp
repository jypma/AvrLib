#ifndef STREAMS_WRITINGHEXADECIMAL_HPP_
#define STREAMS_WRITINGHEXADECIMAL_HPP_

#include "WritingBase.hpp"
#include "Format.hpp"

namespace Streams {
namespace Impl {

static constexpr uint8_t hexChar(uint8_t value) {
    return (value < 10) ? '0' + value : 'A' + (value - 10);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Hexadecimal<uint8_t> value) {
    if (sem::canWrite(fifo, 2)) {
        sem::write(fifo, hexChar(value.value >> 4));
        sem::write(fifo, hexChar(value.value & 0x0F));
        return true;
    } else {
        return false;
    }
}

}
}

#endif /* STREAMS_WRITINGHEXADECIMAL_HPP_ */
