#ifndef STREAMS_WRITINGPADDING_HPP_
#define STREAMS_WRITINGPADDING_HPP_

#include "WritingBase.hpp"
#include "Padding.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, Padding padding) {
    if (sem::canWrite(fifo, padding.getLength())) {
        for (uint8_t i = padding.getLength(); i > 0; i--) {
            sem::write(fifo, uint8_t(0));
        }
        return true;
    } else {
        return false;
    }
}

}
}


#endif /* STREAMS_WRITINGPADDING_HPP_ */
