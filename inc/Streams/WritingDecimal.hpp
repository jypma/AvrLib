#ifndef STREAMS_WRITINGDECIMAL_HPP_
#define STREAMS_WRITINGDECIMAL_HPP_

#include "WritingBase.hpp"
#include "Format.hpp"
#include "EEPROM.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
void writeFunc(void *ctx, uint8_t value) {
    fifo_t &fifo = *((fifo_t*)ctx);

    if (sem::canWrite(fifo, 1)) {
        sem::write(fifo, value);
    }
}

template <typename sem, typename fifo_t, typename int_t>
bool write1(fifo_t &fifo, const Decimal<int_t> value) {
    Format::format(&(writeFunc<sem,fifo_t>), &fifo, value);
    return true;
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint8_t EEPROM::*> field) {
    Format::format(&(writeFunc<sem,fifo_t>), &fifo, dec(read(field.value)));
    return true;
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint16_t EEPROM::*> field) {
    Format::format(&(writeFunc<sem,fifo_t>), &fifo, dec(read(field.value)));
    return true;
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint32_t EEPROM::*> field) {
    Format::format(&(writeFunc<sem,fifo_t>), &fifo, dec(read(field.value)));
    return true;
}

}
}

#endif /* STREAMS_WRITINGDECIMAL_HPP_ */
