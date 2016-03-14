#ifndef STREAMS_READINGNESTED_HPP_
#define STREAMS_READINGNESTED_HPP_

#include "ReadingBase.hpp"
#include "Nested.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t>
struct doread {
    fifo_t *fifo;
    constexpr doread(fifo_t &f): fifo(&f) {}

    template <typename... types>
    ReadResult operator () (types... args) {
        return readN(*fifo, ReadResult::Valid, args...);
    }
};

template <typename fifo_t, typename lambda_t>
ReadResult read1(fifo_t &fifo, Impl::Nested<lambda_t> nested) {
    return nested(doread<fifo_t>(fifo));
}

}
}

#endif /* STREAMS_READINGNESTED_HPP_ */
