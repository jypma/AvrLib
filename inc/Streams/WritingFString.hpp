#ifndef STREAMS_WRITINGFSTRING_HPP_
#define STREAMS_WRITINGFSTRING_HPP_

#include "WritingBase.hpp"
#include "Strings.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
void writeStringInProgmem(fifo_t &fifo, uint8_t *v, uint8_t length) {
    for (uint8_t i = length; i > 0; i--, v++) {
        const uint8_t ch = pgm_read_byte(v);
        sem::write(fifo, ch);
    }
}

template <typename sem, typename fifo_t, uint8_t length>
void write1unchecked(fifo_t &fifo, StringInProgmem<length> *v) {
    writeStringInProgmem<sem>(fifo, (uint8_t*) v, length);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, StringInProgmem<1> *v) {
    const uint8_t ch = pgm_read_byte(v);
    sem::write(fifo, ch);
}

// on AVR, use F("hello") for strings. C-style strings unnecessarily use up RAM.
#ifndef AVR
template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const char *value) {
    while (*value) {
        if (sem::canWrite(fifo, 1)) {
            sem::write(fifo, *value);
        } else {
            return false;
        }
        value++;
    }
    return true;
}
#endif

}
}


#endif /* STREAMS_WRITINGFSTRING_HPP_ */
