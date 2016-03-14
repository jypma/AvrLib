#ifndef STREAMS_WRITINGPROTOCOL_HPP_
#define STREAMS_WRITINGPROTOCOL_HPP_

#include "WritingBase.hpp"
#include "Protocol.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t, typename T>
bool write1(fifo_t &fifo, const T* t, typename T::DefaultProtocol *test = nullptr) {
    return T::DefaultProtocol::template write1<sem>(fifo, t);
}

}
}


#endif /* STREAMS_WRITINGPROTOCOL_HPP_ */
