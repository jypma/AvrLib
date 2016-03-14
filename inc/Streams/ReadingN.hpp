#ifndef STREAMS_READINGN_HPP_
#define STREAMS_READINGN_HPP_

#include "ReadResult.hpp"
#include "gcc_type_traits.h"

namespace Streams {
namespace Impl {

template <typename fifo_t, typename... types>
ReadResult readN(fifo_t &fifo, ReadResult state, types... args);

}
}

#include "ReadingBase.hpp"
#include "ReadingLittleEndian.hpp"
#include "ReadingFString.hpp"
#include "ReadingDecimal.hpp"
#include "ReadingHexadecimal.hpp"
#include "ReadingPadding.hpp"
#include "ReadingNested.hpp"
#include "ReadingChunk.hpp"
#include "ReadingProtocol.hpp"
#include "FixedSizes.hpp"

namespace Streams {
namespace Impl {

template<typename check, typename... types>
struct ReadN {
    // fallback for empty
    static constexpr uint8_t fixedSize = 0;

    template <typename fifo_t>
    static ReadResult apply(fifo_t &fifo, ReadResult state) {
        return state;
    }

    template <typename fifo_t>
    static ReadResult applyUnchecked(fifo_t &fifo, ReadResult state) {
        return state;
    }
};

template<typename head_t, typename... tail_t>
struct ReadN<typename std::enable_if<StreamedSizeReading<head_t>::fixed>::type, head_t, tail_t...> {
    typedef Logging::Log<Loggers::Streams> log;

    // head_t is fixed size
    static constexpr uint8_t fixedSize = StreamedSizeReading<head_t>::size + ReadN<void, tail_t...>::fixedSize;

    template <typename fifo_t>
    static ReadResult apply(fifo_t &fifo, ReadResult state, head_t head, tail_t... tail) {
        if (fifo.getReadAvailable() >= fixedSize) {
            log::debug("space is good, starting to read");
            return applyUnchecked(fifo, state, head, tail...);
        } else {
            return ReadResult::Incomplete;
        }
    }

    template <typename fifo_t>
    static ReadResult applyUnchecked(fifo_t &fifo, ReadResult state, head_t head, tail_t... tail) {
        log::debug("  starting a unchecked read at state %d", state);
        ReadResult s = read1unchecked(fifo, head);
        log::debug("  this read returned %d", s);

        if (state != ReadResult::Invalid || s != ReadResult::Incomplete) {
            // We only allow [state] to be come Incomplete if it's not (yet) Invalid.
            state = s;
        }
        log::debug("  state is now %d", s);
        if (state == ReadResult::Valid) {
            return ReadN<void, tail_t...>::applyUnchecked(fifo, state, tail...);
        } else {
            return state;
        }
    }
};

template<typename head_t, typename... tail_t>
struct ReadN<typename std::enable_if<!StreamedSizeReading<head_t>::fixed>::type, head_t, tail_t...> {
    typedef Logging::Log<Loggers::Streams> log;

    // head_t is not fixed size
    static constexpr uint8_t fixedSize = 0;

    template <typename fifo_t>
    static ReadResult apply(fifo_t &fifo, ReadResult state, head_t head, tail_t... tail) {
        log::debug("  starting a checked read at state %d", state);
        ReadResult s = read1(fifo, head);
        log::debug("  this read returned %d", s);

        if (state != ReadResult::Invalid || s != ReadResult::Incomplete) {
            // We only allow [state] to be come Incomplete if it's not (yet) Invalid.
            state = s;
        }
        log::debug("  state is now %d", s);
        if (state == ReadResult::Valid) {
            return ReadN<void, tail_t...>::template apply(fifo, state, tail...);
        } else {
            return state;
        }
    }

    template <typename fifo_t>
    static ReadResult applyUnchecked(fifo_t &fifo, ReadResult state, head_t head, tail_t... tail) {
        // fall back to checked read, since head_t is not actually a fixed size
        return apply(fifo, state, head, tail...);
    }
};


template <typename fifo_t, typename... types>
ReadResult readN(fifo_t &fifo, ReadResult state, types... args) {
    return ReadN<void, types...>::template apply(fifo, state, args...);
}

}
}



#endif /* STREAMS_READINGN_HPP_ */
