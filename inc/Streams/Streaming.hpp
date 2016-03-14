#ifndef STREAMS_STREAMING_HPP_
#define STREAMS_STREAMING_HPP_

#include "StreamingDecl.hpp"
#include "Reading.hpp"
#include "Writing.hpp"

namespace Streams {
namespace Impl {

template <typename T> template <typename... types>
bool Writing<T>::writeIfSpace(types... args) {
    return ::Streams::Impl::writeIfSpace(*t(), args...);
}

template <typename T> template <typename... types>
void Writing<T>::writeOrBlock(types... args) {
    ::Streams::Impl::writeOrBlock(*t(), args...);
}

template <typename T> template <void (*block)(), typename... types>
void Writing<T>::writeOrBlockWith(types... args) {
    ::Streams::Impl::writeOrBlockWith<block>(*t(), args...);
}

template <typename T> template <typename... types>
Streams::ReadResult Reading<T>::read(types... args) {
    return ::Streams::Impl::read(*t(), args...);
}

}
}



#endif /* STREAMS_STREAMING_HPP_ */
