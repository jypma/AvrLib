#pragma once

#include <stdint.h>
#include "WritingBase.hpp"
#include "Protobuf.hpp"

static inline uint8_t
varint_size(uint32_t v)
{
	if (v < (1UL << 7)) {
		return 1;
	} else if (v < (1UL << 14)) {
		return 2;
	} else if (v < (1UL << 21)) {
		return 3;
	} else if (v < (1UL << 28)) {
		return 4;
	} else {
		return 5;
	}
}

static inline uint8_t
varint_size(uint16_t v)
{
	if (v < (1UL << 7)) {
		return 1;
	} else if (v < (1UL << 14)) {
		return 2;
	} else {
		return 3;
	}
}

static inline uint8_t
varint_size(uint8_t v)
{
	if (v < (1UL << 7)) {
		return 1;
	} else {
		return 2;
	}
}

static inline uint32_t
zigzag(int32_t v)
{
	if (v < 0)
		return (-(uint32_t)v) * 2 - 1;
	else
		return (uint32_t)(v) * 2;
}

static inline uint16_t
zigzag(int16_t v)
{
	if (v < 0)
		return (-(uint16_t)v) * 2 - 1;
	else
		return (uint16_t)(v) * 2;
}

static inline uint8_t
zigzag(int8_t v)
{
	if (v < 0) {
		return (-(uint8_t)v) * 2 - 1;
	} else
		return (uint8_t)(v) * 2;
}

namespace Streams {
namespace Impl {
namespace Protobuf {

enum WireTypes: uint8_t {
	VARINT = 0
};

}

/**
 * Writes an unsigned integer as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<uint32_t, field> v) {
	uint32_t value = v;
	if (sem::canWrite(fifo, varint_size(value) + 1)) {
		sem::write(fifo, field << 3 | Protobuf::VARINT);
		if (value >= 0x80) {
			sem::write(fifo, value | 0x80);
			value >>= 7;
			if (value >= 0x80) {
				sem::write(fifo, value | 0x80);
				value >>= 7;
				if (value >= 0x80) {
					sem::write(fifo, value | 0x80);
					value >>= 7;
					if (value >= 0x80) {
						sem::write(fifo, value | 0x80);
						value >>= 7;
					}
				}
			}
		}
		sem::write(fifo, value);
		return true;
	} else {
		return false;
	}
}

/**
 * Writes an unsigned integer as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<uint16_t, field> v) {
	uint16_t value = v;
	if (sem::canWrite(fifo, varint_size(value) + 1)) {
		sem::write(fifo, field << 3 | Protobuf::VARINT);
		if (value >= 0x80) {
			sem::write(fifo, value | 0x80);
			value >>= 7;
			if (value >= 0x80) {
				sem::write(fifo, value | 0x80);
				value >>= 7;
			}
		}
		sem::write(fifo, value);
		return true;
	} else {
		return false;
	}
}

/**
 * Writes an unsigned integer as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<uint8_t, field> v) {
	uint8_t value = v;
	if (sem::canWrite(fifo, varint_size(value) + 1)) {
		sem::write(fifo, field << 3 | Protobuf::VARINT);
		if (value >= 0x80) {
			sem::write(fifo, value | 0x80);
			value >>= 7;
		}
		sem::write(fifo, value);
		return true;
	} else {
		return false;
	}
}

/**
 * Writes a signed integer with zigzag encoding as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<int32_t, field> v) {
	return write1<sem>(fifo, Protobuf::Varint<uint32_t,field>(zigzag(v)));
}

/**
 * Writes a signed integer with zigzag encoding as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<int16_t, field> v) {
	return write1<sem>(fifo, Protobuf::Varint<uint16_t,field>(zigzag(v)));
}

/**
 * Writes a signed integer with zigzag encoding as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<int8_t, field> v) {
	return write1<sem>(fifo, Protobuf::Varint<uint8_t,field>(zigzag(v)));
}

}
}
