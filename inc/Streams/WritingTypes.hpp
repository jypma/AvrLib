#ifndef STREAMS_WRITINGTYPES_HPP_
#define STREAMS_WRITINGTYPES_HPP_

#include "WritingLittleEndian.hpp"
#include "WritingDecimal.hpp"
#include "WritingNested.hpp"
#include "WritingFString.hpp"
#include "WritingPadding.hpp"
#include "WritingProtocol.hpp"
#include "WritingEEPROMString.hpp"
#include "WritingChunkedFifo.hpp"

namespace Streams {
namespace Impl {

/*
template <typename sem, typename fifo_t, typename T>
bool write1(fifo_t &fifo, T t){
    // If this template is invoked, you've passed an unsupported argument type to Fifo.write().
    return t.unsupported_argument_type_for_write___();
}
*/

}}

#endif /* STREAMS_WRITINGTYPES_HPP_ */
