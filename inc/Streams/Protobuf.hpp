#pragma once

#include "gcc_limits.h"
#include "Varint.hpp"
#include "WritingProtobuf.hpp"
#include "Option.hpp"
#include "Streams/WritingN.hpp"

/**
 * Protobuf implementation that doesn't rely on protoc. Limitiations:
 * - Only varints
 * - Only zigzag encoding for signed ints
 * - Only signed/unsigned ints up to 32 bit
 * - All fields are always optional
 */

namespace Streams {
namespace Protobuf {
namespace ProtocolImpl {

using namespace Streams::Impl;

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

template <typename This, typename... fields>
struct Fields {
    static constexpr uint8_t maxFieldIdx = 0;

    typedef uint8_t presence_t[maxFieldIdx + 1];

    static ReadResult assignField(This *t, uint8_t field, uint32_t result) {
        // no more fields remaining, i.e. ignore this field, since we don't know about it or it's in the wrong format
        return ReadResult::Valid;
    }

    template <typename fifo_t>
    static ReadResult readNestedField(fifo_t &fifo, This *t, uint8_t field, uint32_t length) {
        // no more fields remaining, i.e. ignore this field, since we don't know about it or it's in the wrong format
        return ReadResult::Valid;
    }

    static void initPresence(This *t, uint8_t *p) {}

    static constexpr uint16_t length(const This *t) { return 0; }
};

template <typename This, typename head, typename... tail>
struct Fields<This, head, tail...> {
    static constexpr uint8_t tailMaxFieldIdx = Fields<This, tail...>::maxFieldIdx;
    static constexpr uint8_t maxFieldIdx = (head::fieldIdx > tailMaxFieldIdx) ? head::fieldIdx : tailMaxFieldIdx;

    typedef uint8_t presence_t[maxFieldIdx + 1];

    static void initPresence(This *t, uint8_t *p) {
        head::initialize(t);
        p[head::fieldIdx] = head::initialPresence;
        Fields<This, tail...>::initPresence(t, p);
    }

    static ReadResult assignField(This *t, uint8_t fieldIdx, uint32_t result) {
        return (fieldIdx == head::fieldIdx)
            ? head::assign(t, result)
            : Fields<This, tail...>::assignField(t, fieldIdx, result);
    }

    template <typename fifo_t>
    static ReadResult readNestedField(fifo_t &fifo, This *t, uint8_t fieldIdx, uint32_t length) {
        return (fieldIdx == head::fieldIdx)
            ? head::readNested(fifo, t, length)
            : Fields<This, tail...>::readNestedField(fifo, t, fieldIdx, length);
    }

    static constexpr uint16_t length(const This *t) {
        return head::length(t) + Fields<This, tail...>::length(t);
    }
};

/**
 * Message part that represents an unsigned int as a varint, or a signed int
 * as a zigzag-encoded varint.
 */
template <typename This, uint8_t _fieldIdx, typename T, T This::*field>
struct Varint {
    typedef typename T::unsupported_type_for_varint_encoding foobar;
};

template <typename This, uint8_t _fieldIdx, typename T, T This::*field>
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

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, const uint32_t count) {
        // this field should've been a varint, not a nested other message
        return ReadResult::Invalid;
    }

    static Streams::Protobuf::Varint<T,_fieldIdx> forWriting(const This *t) {
        return t->*field;
    }

    static uint8_t length(const This *t) {
        return varint_size(t->*field) + 1;
    }
};

template <typename This, uint8_t _fieldIdx, typename T, Option<T> This::*field>
struct OptionUnsignedVarint
{
    static constexpr uint8_t fieldIdx = _fieldIdx;
    static constexpr uint8_t initialPresence = 0;

    static void initialize(This *t) {
        (t->*field) = none();
    }

    static ReadResult assign(This *t, uint32_t value) {
        if (value > std::numeric_limits<T>::max()) {
            return ReadResult::Invalid;
        } else {
            (t->*field) = value;
            return ReadResult::Valid;
        }
    }

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, const uint32_t count) {
        // this field should've been a varint, not a nested other message
        return ReadResult::Invalid;
    }

    static auto forWriting(const This *t) {
        return ::Streams::Nested([t] (auto write) {
//                return (t->*field).map([write] (auto i) { return write(i); }).getOrElse(true);
            if ((t->*field).isDefined()) {
                return write(Streams::Protobuf::Varint<T,_fieldIdx>((t->*field).get()));
            } else {
                return true;
            }
        });
    }

    static uint8_t length(const This *t) {
        return ((t->*field).isDefined()) ? varint_size((t->*field).get()) + 1 : 0;
    }
};

template <typename This, uint8_t _fieldIdx, uint8_t This::*field>
struct Varint<This, _fieldIdx, uint8_t, field>: public UnsignedVarint<This, _fieldIdx, uint8_t, field> {};

template <typename This, uint8_t _fieldIdx, uint16_t This::*field>
struct Varint<This, _fieldIdx, uint16_t, field>: public UnsignedVarint<This, _fieldIdx, uint16_t, field> {};

template <typename This, uint8_t _fieldIdx, uint32_t This::*field>
struct Varint<This, _fieldIdx, uint32_t, field>: public UnsignedVarint<This, _fieldIdx, uint32_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<uint8_t> This::*field>
struct Varint<This, _fieldIdx, Option<uint8_t>, field>: public OptionUnsignedVarint<This, _fieldIdx, uint8_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<uint16_t> This::*field>
struct Varint<This,_fieldIdx, Option<uint16_t>, field>: public OptionUnsignedVarint<This, _fieldIdx, uint16_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<uint32_t> This::*field>
struct Varint<This, _fieldIdx, Option<uint32_t>, field>: public OptionUnsignedVarint<This, _fieldIdx, uint32_t, field> {};

template <typename This, uint8_t _fieldIdx, typename T, T This::*field>
struct SignedVarint
{
    static constexpr uint8_t fieldIdx = _fieldIdx;
    static constexpr uint8_t initialPresence = 1;

    static void initialize(This *t) {}

    static ReadResult assign(This *t, uint32_t value) {
        int32_t v = unzigzag32(value);
        if (v > std::numeric_limits<T>::max() || v < std::numeric_limits<T>::min()) {
            return ReadResult::Invalid;
        } else {
            (t->*field) = v;
            return ReadResult::Valid;
        }
    }

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, const uint32_t count) {
        // this field should've been a varint, not a nested other message
        return ReadResult::Invalid;
    }

    static Streams::Protobuf::Varint<T,_fieldIdx> forWriting(const This *t) {
        return t->*field;
    }

    static uint8_t length(const This *t) {
        return varint_size(zigzag(t->*field)) + 1;
    }
};

template <typename This, uint8_t _fieldIdx, typename T, Option<T> This::*field>
struct OptionSignedVarint
{
    static constexpr uint8_t fieldIdx = _fieldIdx;
    static constexpr uint8_t initialPresence = 0;

    static void initialize(This *t) {
        (t->*field) = none();
    }

    static ReadResult assign(This *t, uint32_t value) {
        int32_t v = unzigzag32(value);
        if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min()) {
            return ReadResult::Invalid;
        } else {
            (t->*field) = value;
            return ReadResult::Valid;
        }
    }

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, const uint32_t count) {
        // this field should've been a varint, not a nested other message
        return ReadResult::Invalid;
    }

    static auto forWriting(const This *t) {
        return ::Streams::Nested([t] (auto write) {
            //return (t->*field).map([write] (auto i) { return write(i); }).getOrElse(true);
            if ((t->*field).isDefined()) {
                return write(Streams::Protobuf::Varint<T,_fieldIdx>((t->*field).get()));
            } else {
                return true;
            }
        });
    }

    static uint8_t length(const This *t) {
        return ((t->*field).isDefined()) ? varint_size(zigzag((t->*field).get())) + 1 : 0;
    }
};

template <typename This, uint8_t _fieldIdx, int8_t This::*field>
struct Varint<This, _fieldIdx, int8_t, field>: public SignedVarint<This, _fieldIdx, int8_t, field> {};

template <typename This, uint8_t _fieldIdx, int16_t This::*field>
struct Varint<This, _fieldIdx, int16_t, field>: public SignedVarint<This, _fieldIdx, int16_t, field> {};

template <typename This, uint8_t _fieldIdx, int32_t This::*field>
struct Varint<This, _fieldIdx, int32_t, field>: public SignedVarint<This, _fieldIdx, int32_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<int8_t> This::*field>
struct Varint<This, _fieldIdx, Option<int8_t>, field>: public OptionSignedVarint<This, _fieldIdx, int8_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<int16_t> This::*field>
struct Varint<This, _fieldIdx, Option<int16_t>, field>: public OptionSignedVarint<This, _fieldIdx, int16_t, field> {};

template <typename This, uint8_t _fieldIdx, Option<int32_t> This::*field>
struct Varint<This, _fieldIdx, Option<int32_t>, field>: public OptionSignedVarint<This, _fieldIdx, int32_t, field> {};

template <typename This, uint8_t _fieldIdx, typename U, U This::*field, typename protocol = typename U::DefaultProtocol>
struct SubMessage {
    static constexpr uint8_t fieldIdx = _fieldIdx;
    static constexpr uint8_t initialPresence = 1;

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, uint32_t length) {
        return protocol::readNested(fifo, &(t->*field), length);
    }

    static ReadResult assign(This *t, uint32_t value) {
        // This field index should've been a sub-message, not a varint value
        return ReadResult::Invalid;
    }

    static void initialize(This *t) {}

    static auto forWriting(const This *t) {
        return protocol::template forWriting<_fieldIdx>(&(t->*field));
    };

    static uint8_t length(const This *t) {
        return protocol::length(&(t->*field));
    }
};

/**
 * A protocol that represents an undelimited or delimited protobuf message.
 * When reading an initial protobug message, read will continue until the end of the fifo or chunk.
 * This type is also used to indicate nested protobuf messages, which ARE prefixed with their length.
 */
template <typename This, typename... fields>
class Message {
    using F = Fields<This, fields...>;
public:
    template <typename fifo_t>
    static ReadResult read1(fifo_t &fifo, This *t) {
        typename F::presence_t presence = {};
        F::initPresence(t, presence);
        while (fifo.getReadAvailable() >= 2) {
            uint8_t field_and_type;
            fifo.uncheckedRead(field_and_type);
            if ((field_and_type & 0x07) == VARINT) {
                uint32_t value;
                ReadResult result = readVarint(fifo, value);
                if (result != ReadResult::Valid) {
                    return result;
                }
                const uint8_t fieldIdx = field_and_type >> 3;
                result = F::assignField(t, fieldIdx, value);
                if (result != ReadResult::Valid) {
                    return result;
                } else {
                    if (fieldIdx <= F::maxFieldIdx) {
                        presence[fieldIdx] = 0;
                    }
                }
            } else if ((field_and_type & 0x07) == LENGTH_DELIMITED) {
                uint32_t length;
                ReadResult result = readVarint(fifo, length);
                if (result != ReadResult::Valid) {
                    return result;
                }
                const uint8_t fieldIdx = field_and_type >> 3;
                result = F::readNestedField(fifo, t, fieldIdx, length);
                if (result != ReadResult::Valid) {
                    return result;
                } else {
                    if (fieldIdx <= F::maxFieldIdx) {
                        presence[fieldIdx] = 0;
                    }
                }
            } else {
                return ReadResult::Invalid;
            }
        }
        bool complete = true;
        for (uint8_t i = 0; i <= F::maxFieldIdx; i++) {
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

    template <typename fifo_t>
    static ReadResult readNested(fifo_t &fifo, This *t, const uint32_t count) {
        typename F::presence_t presence = {};
        F::initPresence(t, presence);
        if (fifo.getReadAvailable() < count) {
            return ReadResult::Partial;
        }
        if (count > 0x7FFF) {
            return ReadResult::Invalid;
        }
        int16_t remaining = count;
        while (remaining > 0) {
            uint8_t field_and_type;
            fifo.uncheckedRead(field_and_type);
            remaining--;
            if ((field_and_type & 0x07) == VARINT) {
                uint32_t value;
                uint8_t before = fifo.getReadAvailable();
                ReadResult result = readVarint(fifo, value);
                remaining -= (before - fifo.getReadAvailable());
                if (result != ReadResult::Valid) {
                    return result;
                }
                const uint8_t fieldIdx = field_and_type >> 3;
                result = F::assignField(t, fieldIdx, value);
                if (result != ReadResult::Valid) {
                    return result;
                } else {
                    if (fieldIdx <= F::maxFieldIdx) {
                        presence[fieldIdx] = 0;
                    }
                }
            } else if ((field_and_type & 0x07) == LENGTH_DELIMITED) {
                uint32_t length;
                uint8_t before = fifo.getReadAvailable();
                ReadResult result = readVarint(fifo, length);
                if (result != ReadResult::Valid) {
                    return result;
                }
                const uint8_t fieldIdx = field_and_type >> 3;
                result = F::readNestedField(fifo, t, fieldIdx, length);
                remaining -= (before - fifo.getReadAvailable());
                if (result != ReadResult::Valid) {
                    return result;
                } else {
                    if (fieldIdx <= F::maxFieldIdx) {
                        presence[fieldIdx] = 0;
                    }
                }
            } else {
                return ReadResult::Invalid;
            }
        }
        if (remaining < 0) { // we got more bytes than the initial length indicated
            return ReadResult::Invalid;
        }
        bool complete = true;
        for (uint8_t i = 0; i <= F::maxFieldIdx; i++) {
            if (presence[i] == 1) {
                complete = false;
                break;
            }
        }
        if (!complete) {
            return ReadResult::Incomplete;
        } else {
            return ReadResult::Valid;
        }
    }

    template <typename sem, typename fifo_t>
    static bool write1(fifo_t &fifo, const This *t) {
        return ::Streams::Impl::writeN<sem>(fifo, fields::forWriting(t)...);
    }

    template <uint8_t fieldIdx>
    static auto forWriting(const This *t) {
        return ::Streams::Nested([t] (auto write) {
            uint16_t l = F::length(t);
            return write(
                uint8_t(fieldIdx << 3 | LENGTH_DELIMITED),
                Streams::Protobuf::BareVarint<uint16_t>(l),
                fields::forWriting(t)...
            );
        });
    }

    static uint16_t length(const This *t) {
        uint16_t l = F::length(t);
        return varint_size(l) + l + 1;
    }
};

} // namespace ProtocolImpl

template <typename This>
struct Protocol {
    template <typename... fields>
    using Message = ProtocolImpl::Message<This, fields...>;

    template <uint8_t _fieldIdx, typename T, T This::*field>
    using Varint = ProtocolImpl::Varint<This, _fieldIdx, T, field>;

    template <uint8_t _fieldIdx, typename U, U This::*field, typename protocol = typename U::DefaultProtocol>
    using SubMessage = ProtocolImpl::SubMessage<This, _fieldIdx, U, field, protocol>;
};

} // namespace Protobuf
} // namespace Streams



