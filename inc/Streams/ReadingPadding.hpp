#ifndef STREAMS_READINGPADDING_HPP_
#define STREAMS_READINGPADDING_HPP_

#include "ReadingBase.hpp"
#include "Padding.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t>
ReadResult read1(fifo_t &fifo, Padding p) {
    if (fifo.getReadAvailable() < p.getLength()) {
        return ReadResult::Incomplete;
    } else {
        for (uint8_t i = p.getLength(); i > 0; i--) {
            uint8_t dummy;
            fifo.uncheckedRead(dummy);
        }
        return ReadResult::Valid;
    }
}

}
}



#endif /* STREAMS_READINGPADDING_HPP_ */
