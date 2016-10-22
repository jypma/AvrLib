#ifndef STREAMS_READRESULT_HPP_
#define STREAMS_READRESULT_HPP_

#include <stdint.h>
#include "Enum.hpp"

namespace Streams {

namespace E {
    struct ReadResult {
        enum type: uint8_t {
            /**
             * The read was successful: all elements have been read.
             */
            Valid,
            /**
             * The read was unsuccessful, because the encountered data did not match the expected data format.
             * A future read with more data will always also fail with Invalid.
             */
            Invalid,
            /**
             * The read was unsuccessful, because not enough data was encountered.
             * A future read with more data _might_ become Valid (but might also become Invalid, depending on the data).
             */
            Incomplete,
            /**
             * The read was unsuccessful, because not enough data was encountered. However, one read element was partially read.
             * A future read with more data _might_ become Valid (but might also become Invalid, depending on the data).
             */
            Partial
        };
    };
}

class ReadResult: public Enum<E::ReadResult> {
public:
    using Enum<E::ReadResult>::Enum;

    operator bool() const {
        return underlying() == E::ReadResult::type::Valid;
    }
};

#ifndef AVR
inline ::std::ostream& operator<<(::std::ostream& os, const ReadResult& that) {
    return os << that.underlying();
}
#endif

}



#endif /* STREAMS_READRESULT_HPP_ */
