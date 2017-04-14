#pragma once

#include <stdint.h>
#include "Varint.hpp"
#include "WritingBase.hpp"
#include "Protobuf.hpp"

namespace Streams {
namespace Impl {

/**
 * Writes an unsigned integer as a protobuf varint, with the given field index.
 */
template <typename sem, typename fifo_t, uint8_t field>
bool write1(fifo_t &fifo, const Protobuf::Varint<uint32_t, field> v) {
	uint32_t value = v;
	if (sem::canWrite(fifo, Protobuf::varint_size(value) + 1)) {
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
	if (sem::canWrite(fifo, Protobuf::varint_size(value) + 1)) {
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

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Protobuf::BareVarint<uint16_t> v) {
    uint16_t value = v;
    if (sem::canWrite(fifo, Protobuf::varint_size(value))) {
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
	if (sem::canWrite(fifo, Protobuf::varint_size(value) + 1)) {
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
