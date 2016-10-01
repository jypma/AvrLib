#ifndef STREAMS_WRITINGDECIMAL_HPP_
#define STREAMS_WRITINGDECIMAL_HPP_

#include "WritingBase.hpp"
#include "Format.hpp"
#include "EEPROM.hpp"
#include "Option.hpp"

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
bool writeFunc(void *ctx, uint8_t value) {
    fifo_t &fifo = *((fifo_t*)ctx);

    if (sem::canWrite(fifo, 1)) {
        sem::write(fifo, value);
        return true;
    } else {
        return false;
    }
}

template <typename sem, typename fifo_t, typename int_t>
bool write1decimalInt(fifo_t &fifo, const Decimal<int_t> value) {
    return Format::format(&(writeFunc<sem,fifo_t>), &fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint8_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<bool> value) {
    if (sem::canWrite(fifo, 1)) {
        sem::write(fifo, value.value ? '1' : '0');
        return true;
    } else {
        return false;
    }
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<int8_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint16_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<int16_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<uint32_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<int32_t> value) {
    return write1decimalInt<sem>(fifo, value);
}

template <typename sem, typename fifo_t, typename read_delegate_t>
bool write1(fifo_t &fifo, const Decimal<read_delegate_t*> d) {
    if (d.value->isReading()) {
        const uint8_t count = d.value->getReadAvailable();
        for (uint8_t i = count; i > 0; i--) {
            uint8_t item;
            d.value->uncheckedRead(item);
            if (!write1decimalInt<sem>(fifo, dec(item))) return false;
            if (i > 1) {
                if (sem::canWrite(fifo, 1)) {
                    sem::write(fifo, ',');
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

template <typename sem, typename fifo_t, typename int_t>
bool write1(fifo_t &fifo, const Decimal<ArraySlice<int_t>> d) {
    for (uint8_t i = d.value.first; i < d.value.until; i++) {
        if (i > d.value.first) {
            if (sem::canWrite(fifo, 1)) {
                sem::write(fifo, ',');
            } else {
                return false;
            }
        }
        if (!write1decimalInt<sem>(fifo, dec(d.value.array[i]))) return false;
    }
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

template <typename sem, typename fifo_t, typename T>
bool write1(fifo_t &fifo, const Decimal<Option<T>> optValue) {
	// can't map here, since capturing fifo results in unpacked non-POD field, even though the whole thing is force inlined.
	//return optValue.value.map([&] (auto v) { return write1decimalInt<sem>(fifo, Decimal<T>(v)); }).getOrElse(false);
	if (optValue.value.isDefined()) {
		return write1decimalInt<sem>(fifo, Decimal<T>(optValue.value.get()));
	} else {
		return false;
	}
}

}
}

#endif /* STREAMS_WRITINGDECIMAL_HPP_ */
