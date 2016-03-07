#ifndef STREAMS_READINGLITTLEENDIAN_HPP_
#define STREAMS_READINGLITTLEENDIAN_HPP_

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif


#include "ReadingBase.hpp"

namespace Streams {
namespace Impl {

template <uint8_t size, typename fifo_t>
inline ReadResult __attribute__((optimize("unroll-loops"))) readLiteralBytes(fifo_t &fifo, void *value) {
    for (uint8_t i = 0; i < size; i++) {
        fifo.uncheckedRead( ((uint8_t*)value)[i] );
    }
    return ReadResult::Valid;
}

template <typename fifo_t, typename T>
ReadResult readLiteral(fifo_t &fifo, T *value) {
    return readLiteralBytes<sizeof(T)>(fifo, value);
}

template <typename fifo_t, typename enum_t>
ReadResult read1unchecked(fifo_t &fifo, enum_t *v, typename enum_t::type *test = nullptr) {
    return readLiteral(fifo, v->underlying_ptr());
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, bool *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t, uint8_t count>
ReadResult read1unchecked(fifo_t &fifo, uint8_t (*v) [count]) {
    return readLiteralBytes<count>(fifo, *v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, uint8_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, int8_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, uint16_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, int16_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, uint32_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, int32_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, uint64_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, int64_t *v) {
    return readLiteral(fifo, v);
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, char *v) {
    return readLiteral(fifo, v);
}

} // namespace Impl
} // namespace Streams


#endif /* STREAMS_READINGLITTLEENDIAN_HPP_ */
