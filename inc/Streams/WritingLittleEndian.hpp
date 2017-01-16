#ifndef STREAMS_WRITINGLITTLEENDIAN_HPP_
#define STREAMS_WRITINGLITTLEENDIAN_HPP_

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif


#include "EEPROM.hpp"
#include <stdint.h>
#include "gcc_type_traits.h"

namespace Streams {
namespace Impl {

//FIXME provide manual unrolled loops for sizes 1,2,4, and decide which of those to force_inline
template <typename sem, typename fifo_t>
void writeLiteralBytes(fifo_t &fifo, const uint8_t *src, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        sem::write(fifo, src[i] );
    }
}

template <typename sem, typename fifo_t, uint8_t count>
struct fn_writeLiteralBytes {
	static inline void apply(fifo_t &fifo, const uint8_t *src) {
		writeLiteralBytes<sem>(fifo, src, count);
	}
};

template <typename sem, typename fifo_t>
struct fn_writeLiteralBytes<sem,fifo_t,1> {
	static inline void apply(fifo_t &fifo, const uint8_t *src) {
		sem::write(fifo, src[0]);
	}
};

template <typename sem, typename fifo_t>
struct fn_writeLiteralBytes<sem,fifo_t,2> {
	static inline void apply(fifo_t &fifo, const uint8_t *src) {
		sem::write(fifo, src[0]);
		sem::write(fifo, src[1]);
	}
};

template <typename sem, typename fifo_t, typename T, typename check=void>
struct fn_writeLiteralByteValue {
	static inline void apply(fifo_t &fifo, const T value) {
		writeLiteralBytes<sem,fifo_t>(fifo, (uint8_t*) &value, sizeof(T));
	}
};

template <typename sem, typename fifo_t, typename T>
struct fn_writeLiteralByteValue<sem,fifo_t,T,typename std::enable_if<sizeof(T) == 1>::type> {
	static inline void apply(fifo_t &fifo, const T value) {
		const uint8_t * const src = (uint8_t*) &value;
		sem::write(fifo, src[0]);
	}
};

template <typename sem, typename fifo_t, typename T>
struct fn_writeLiteralByteValue<sem,fifo_t,T,typename std::enable_if<sizeof(T) == 2>::type> {
	static inline void apply(fifo_t &fifo, const T value) {
		const uint8_t * const src = (uint8_t*) &value;
		sem::write(fifo, src[0]);
		sem::write(fifo, src[1]);
	}
};

template <typename sem, typename fifo_t, typename T>
inline void writeLiteralByteValue(fifo_t &fifo, const T value) {
	fn_writeLiteralByteValue<sem,fifo_t,T>::apply(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const bool value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t, typename enum_t>
void write1unchecked(fifo_t &fifo, enum_t v, typename enum_t::type *test = nullptr) {
    writeLiteralByteValue<sem>(fifo, v.underlying());
}

template <typename sem, typename fifo_t, uint8_t count>
void write1unchecked(fifo_t &fifo, const uint8_t (*value) [count]) {
    fn_writeLiteralBytes<sem, fifo_t, count>::apply(fifo, *value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const uint8_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const int8_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const uint16_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const int16_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const uint32_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const int32_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const uint64_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const int64_t value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const char value) {
    writeLiteralByteValue<sem>(fifo, value);
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, uint8_t EEPROM::*field) {
    writeLiteralByteValue<sem>(fifo, read(field));
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, uint16_t EEPROM::*field) {
    writeLiteralByteValue<sem>(fifo, read(field));
}

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, uint32_t EEPROM::*field) {
    writeLiteralByteValue<sem>(fifo, read(field));
}

}
}



#endif /* STREAMS_WRITINGLITTLEENDIAN_HPP_ */
