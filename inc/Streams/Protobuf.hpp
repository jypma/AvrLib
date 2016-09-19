#pragma once

#include "gcc_limits.h"
#include "WritingProtobuf.hpp"

/**
 * Protobuf implementation that doesn't rely on protoc. Limitiations:
 * - Only varints
 * - Only zigzag encoding for signed ints
 * - Only signed/unsigned ints up to 32 bit
 * - All fields are always optional
 */

namespace Streams {
namespace Impl {
namespace Protobuf {

template <typename int_t, uint8_t _fieldIdx>
class Varint {
	static_assert(_fieldIdx < (0xFF >> 3), "field numbers must fit in 5 bits");
	int_t value;
public:
	static constexpr uint8_t fieldIdx = _fieldIdx;

	constexpr Varint(int_t v): value(v) {}
	constexpr operator int_t() const { return value; }
};

static inline int32_t
unzigzag32(uint32_t v)
{
	if (v & 1)
		return -(v >> 1) - 1;
	else
		return v >> 1;
}

template <typename fifo_t>
ReadResult readVarint(fifo_t &fifo, uint32_t &value) {
	uint8_t avail = fifo.getReadAvailable();
	if (avail == 0) {
		return ReadResult::Incomplete;
	}
	uint8_t data;
	fifo.uncheckedRead(data);
	avail--;
	value = data & 0x7f;
	if (data & 0x80) {
		if (avail == 0) {
			return ReadResult::Partial;
		}
		fifo.uncheckedRead(data);
		avail--;
		value |= ((uint32_t) (data & 0x7f) << 7);
		if (data & 0x80) {
			if (avail == 0) {
				return ReadResult::Partial;
			}
			fifo.uncheckedRead(data);
			avail--;
			value |= ((uint32_t) (data & 0x7f) << 14);
			if (data & 0x80) {
				if (avail == 0) {
					return ReadResult::Partial;
				}
				fifo.uncheckedRead(data);
				avail--;
				value |= ((uint32_t) (data & 0x7f) << 21);
				if (data & 0x80) {
					if (avail == 0) {
						return ReadResult::Partial;
					}
					fifo.uncheckedRead(data);
					if (data & 0x80) {
						return ReadResult::Invalid;
					} else {
						value |= ((uint32_t) (data) << 28);
					}
				}
			}
		}
	}
	return ReadResult::Valid;
}

}
}

namespace Protobuf {

template <typename This>
struct Protocol {

	template <typename... fields>
	struct Fields {
		static constexpr uint8_t maxFieldIdx = 0;

		static ReadResult assignField(This *t, uint8_t field, uint32_t result) {
			// no more fields remaining, i.e. ignore this field
			return ReadResult::Valid;
		}

		static void initPresence(This *t, uint8_t *p) {}
	};

	template <typename head, typename... tail>
	struct Fields<head, tail...> {
		static constexpr uint8_t tailMaxFieldIdx = Fields<tail...>::maxFieldIdx;
		static constexpr uint8_t maxFieldIdx = (head::fieldIdx > tailMaxFieldIdx) ? head::fieldIdx : tailMaxFieldIdx;

		typedef uint8_t presence_t[maxFieldIdx + 1];

		static void initPresence(This *t, uint8_t *p) {
			head::initialize(t);
			p[head::fieldIdx] = head::initialPresence;
			Fields<tail...>::initPresence(t, p);
		}

		static ReadResult assignField(This *t, uint8_t fieldIdx, uint32_t result) {
			return (fieldIdx == head::fieldIdx)
				? head::assign(t, result)
				: Fields<tail...>::assignField(t, fieldIdx, result);
		}
	};

	/**
	 * Marks field F, itself probably a Varint<...>, as optional, storing in [present]
	 * whether the field is/was present or not.
	 */
	template <bool This::*present, typename F>
	struct Optional {
		static constexpr uint8_t fieldIdx = F::fieldIdx;
		static constexpr uint8_t initialPresence = 0;

		static void initialize(This *t) {
			(t->*present) = false;
		}

		static ReadResult assign(This *t, uint32_t value) {
			auto result = F::assign(t, value);
			(t->*present) = (result == ReadResult::Valid);
			return result;
		}

		static auto forWriting(const This *t) {
            return ::Streams::Nested([t] (auto write) {
                return (t->*present) ? write(F::forWriting(t)) : true;
            });
		}
	};

	/**
	 * Message part that represents an unsigned int as a varint, or a signed int
	 * as a zigzag-encoded varint.
	 */
	template <uint8_t _fieldIdx, typename T, T This::*field>
	struct Varint;

	template <uint8_t _fieldIdx, typename T, T This::*field>
	struct UnsignedVarint
	{
		static constexpr uint8_t fieldIdx = _fieldIdx;
		static constexpr uint8_t initialPresence = 1;

		static void initialize(This *t) {}

		static ReadResult assign(This *t, uint32_t value) {
			if (value > std::numeric_limits<T>::max()) {
				return ReadResult::Invalid;
			} else {
				(t->*field) = value;
				return ReadResult::Valid;
			}
		}

		static Impl::Protobuf::Varint<T,_fieldIdx> forWriting(const This *t) {
			return t->*field;
		}
	};

	template <uint8_t _fieldIdx, uint8_t This::*field>
	struct Varint<_fieldIdx, uint8_t, field>: public UnsignedVarint<_fieldIdx, uint8_t, field> {};

	template <uint8_t _fieldIdx, uint16_t This::*field>
	struct Varint<_fieldIdx, uint16_t, field>: public UnsignedVarint<_fieldIdx, uint16_t, field> {};

	template <uint8_t _fieldIdx, uint32_t This::*field>
	struct Varint<_fieldIdx, uint32_t, field>: public UnsignedVarint<_fieldIdx, uint32_t, field> {};

	template <uint8_t _fieldIdx, typename T, T This::*field>
	struct SignedVarint
	{
		static constexpr uint8_t fieldIdx = _fieldIdx;
		static constexpr uint8_t initialPresence = 1;

		static void initialize(This *t) {}

		static ReadResult assign(This *t, uint32_t value) {
			int32_t v = Impl::Protobuf::unzigzag32(value);
			if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min()) {
				return ReadResult::Invalid;
			} else {
				(t->*field) = value;
				return ReadResult::Valid;
			}
		}

		static Impl::Protobuf::Varint<T,_fieldIdx> forWriting(const This *t) {
			return t->*field;
		}
	};

	template <uint8_t _fieldIdx, int8_t This::*field>
	struct Varint<_fieldIdx, int8_t, field>: public SignedVarint<_fieldIdx, int8_t, field> {};

	template <uint8_t _fieldIdx, int16_t This::*field>
	struct Varint<_fieldIdx, int16_t, field>: public SignedVarint<_fieldIdx, int16_t, field> {};

	template <uint8_t _fieldIdx, int32_t This::*field>
	struct Varint<_fieldIdx, int32_t, field>: public SignedVarint<_fieldIdx, int32_t, field> {};

	/**
	 * A protocol that represents an undelimited protobuf message. When reading,
	 * read will continue until the end of the fifo or chunk.
	 */
	//FIXME merge Fields into this as private
	template <typename... fields>
	struct Message {
        template <typename fifo_t>
        static ReadResult read1(fifo_t &fifo, This *t) {
        	typename Fields<fields...>::presence_t presence = {};
        	Fields<fields...>::initPresence(t, presence);
        	while (fifo.getReadAvailable() >= 2) {
        		uint8_t field_and_type;
        		fifo.uncheckedRead(field_and_type);
        		if ((field_and_type & 0x07) == 0 /* VARINT */) {
        			uint32_t value;
        			ReadResult result = ::Streams::Impl::Protobuf::readVarint(fifo, value);
        			if (result != ReadResult::Valid) {
        				return result;
        			}
        			const uint8_t fieldIdx = field_and_type >> 3;
        			result = Fields<fields...>::assignField(t, fieldIdx, value);
        			if (result != ReadResult::Valid) {
        				return result;
        			} else {
        				if (fieldIdx <= Fields<fields...>::maxFieldIdx) {
        					presence[fieldIdx] = 0;
        				}
        			}
        		} else {
        			return ReadResult::Invalid;
        		}
        	}
        	bool complete = true;
        	for (uint8_t i = 0; i <= Fields<fields...>::maxFieldIdx; i++) {
        		if (presence[i] == 1) {
        			complete = false;
        			break;
        		}
        	}
        	if (fifo.getReadAvailable() == 1) {
				return ReadResult::Partial;
			} else if (!complete) {
        		return ReadResult::Incomplete;
        	} else {
        		return ReadResult::Valid;
        	}
        }

        template <typename sem, typename fifo_t>
        static bool write1(fifo_t &fifo, const This *t) {
            return ::Streams::Impl::writeN<sem>(fifo, fields::forWriting(t)...);
        }

	};
};

template<uint8_t fieldIdx, typename int_t>
constexpr Impl::Protobuf::Varint<int_t,fieldIdx> Varint(int_t i) { return i; }

}
}



