#ifndef STREAMS_READINGBASE_HPP_
#define STREAMS_READINGBASE_HPP_

#include "ReadResult.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t, typename T>
ReadResult read1(fifo_t &fifo, T t) {
    // If this template is invoked, you've passed an unsupported argument type to Fifo.read().
    return t.unsupported_argument_type_for_read___();
}

template <typename fifo_t, typename T>
ReadResult read1unchecked(fifo_t &fifo, T t) {
    // If this template is invoked, you've passed an unsupported argument type to Fifo.read().
    return t.unsupported_argument_type_for_read___();
}

}}

#endif /* STREAMS_READINGBASE_HPP_ */
