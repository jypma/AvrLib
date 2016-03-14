#ifndef STREAMS_READINGPROTOCOL_HPP_
#define STREAMS_READINGPROTOCOL_HPP_

#include "ReadingBase.hpp"
#include "Protocol.hpp"

namespace Streams {
namespace Impl {

template <typename fifo_t, typename T>
ReadResult read1(fifo_t &fifo, T *t, typename T::DefaultProtocol *test = nullptr) {
    return  T::DefaultProtocol::read1(fifo, t);
}

}
}



#endif /* STREAMS_READINGPROTOCOL_HPP_ */
