#ifndef STREAMS_WRITINGFSTRING_HPP_
#define STREAMS_WRITINGFSTRING_HPP_

#include "WritingBase.hpp"
#include "Strings.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t, uint8_t length>
bool write1(fifo_t &fifo, StringInProgmem<length> *v) {
    if (sem::canWrite(fifo, length)) {
        for (uint8_t i = length; i > 0; i--, v++) {
            const uint8_t ch = pgm_read_byte(v);
            sem::write(fifo, ch);
        }
        return true;
    } else {
        return false;
    }
}

}
}


#endif /* STREAMS_WRITINGFSTRING_HPP_ */
