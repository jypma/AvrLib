#ifndef STREAMS_WRITINGN_HPP_
#define STREAMS_WRITINGN_HPP_

#include "gcc_type_traits.h"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t, typename... types>
bool writeN(fifo_t &fifo, types... args);

}
}

#include "WritingBase.hpp"
#include "WritingLittleEndian.hpp"
#include "WritingDecimal.hpp"
#include "WritingHexadecimal.hpp"
#include "WritingNested.hpp"
#include "WritingFString.hpp"
#include "WritingPadding.hpp"
#include "WritingEEPROMString.hpp"
#include "WritingProtobuf.hpp"
#include "WritingChunkedFifo.hpp"
#include "WritingProtocol.hpp"
#include "FixedSizes.hpp"

namespace Streams {
namespace Impl {

template<typename check, typename... types>
struct WriteN {
    // fallback for empty
    static constexpr uint8_t fixedSize = 0;

    template <typename sem, typename fifo_t>
    static bool apply(fifo_t &fifo) {
        return true;
    }

    template <typename sem, typename fifo_t>
    static bool applyUnchecked(fifo_t &fifo) {
        return true;
    }
};

template<typename head_t, typename... tail_t>
struct WriteN<typename std::enable_if<StreamedSizeWriting<head_t>::fixed>::type, head_t, tail_t...> {

    // head_t is fixed size
    static constexpr uint8_t fixedSize = StreamedSizeWriting<head_t>::size + WriteN<void, tail_t...>::fixedSize;

    template <typename sem, typename fifo_t>
    static bool apply(fifo_t &fifo, head_t head, tail_t... tail) {
        if (sem::canWrite(fifo, fixedSize)) {
            return applyUnchecked<sem>(fifo, head, tail...);
        } else {
            return false;
        }
    }

    template <typename sem, typename fifo_t>
    static bool applyUnchecked(fifo_t &fifo, head_t head, tail_t... tail) {
        write1unchecked<sem>(fifo, head);
        return WriteN<void,tail_t...>::template applyUnchecked<sem>(fifo, tail...);
    }
};

template<typename head_t, typename... tail_t>
struct WriteN<typename std::enable_if<!StreamedSizeWriting<head_t>::fixed>::type, head_t, tail_t...> {
    // head_t is not fixed size
    static constexpr uint8_t fixedSize = 0;

    template <typename sem, typename fifo_t>
    static bool apply(fifo_t &fifo, head_t head, tail_t... tail) {
        if (write1<sem>(fifo, head)) {
            return WriteN<void,tail_t...>::template apply<sem>(fifo, tail...);
        } else {
            return false;
        }
    }

    template <typename sem, typename fifo_t>
    static bool applyUnchecked(fifo_t &fifo, head_t head, tail_t... tail) {
        return apply<sem>(fifo, head, tail...);
    }

};

template <typename sem, typename fifo_t, typename... types>
bool writeN(fifo_t &fifo, types... args) {
    return WriteN<void, types...>::template apply<sem>(fifo, args...);
}

}
}



#endif /* STREAMS_WRITINGN_HPP_ */
