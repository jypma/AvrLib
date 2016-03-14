#ifndef STREAMS_WRITINGNESTED_HPP_
#define STREAMS_WRITINGNESTED_HPP_

#include "WritingBase.hpp"
#include "Nested.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
struct dowrite {
    fifo_t *fifo;
    constexpr dowrite(fifo_t &f): fifo(&f) {}

    template <typename... types>
    bool operator () (types... args) {
        return writeN<sem>(*fifo, args...);
    }
};

template <typename sem, typename fifo_t, typename lambda_t>
bool write1(fifo_t &fifo, Impl::Nested<lambda_t> nested) {
    return nested(dowrite<sem, fifo_t>(fifo));
}

}
}



#endif /* STREAMS_WRITINGNESTED_HPP_ */
