#ifndef STREAMS_PROTOCOL_HPP_
#define STREAMS_PROTOCOL_HPP_

#include "ReadResult.hpp"

namespace Streams {
namespace Impl {

template <typename T, typename P>
struct WithProtocol {
    T * const t;
    WithProtocol(T *_t): t(_t) {}
};

template <typename T, typename P>
struct WithProtocolConst {
    const T * const t;
    WithProtocolConst(const T *_t): t(_t) {}
};

template <typename fifo_t, typename T, typename P>
ReadResult read1(fifo_t &fifo, WithProtocol<T, P> wrapper) {
    return P::read1(fifo, wrapper.t);
}

template <typename sem, typename fifo_t, typename T, typename P>
bool write1(fifo_t &fifo, WithProtocolConst<T, P> wrapper) {
    return P::template write1<sem>(fifo, wrapper.t);
}

} // namespace Streams
} // namespace Impl

#include "ReadingN.hpp"
#include "WritingN.hpp"
#include "Format.hpp"

namespace Streams {

template <typename This>
struct Protocol {
    /** Implementation-private. Do not use. */
    template <typename element>
    struct Single {
        template <typename fifo_t>
        static ReadResult read1(fifo_t &fifo, This *t) {
            return Impl::readN(fifo, ReadResult::Valid, element::forReading(t));
        }

        template <typename sem, typename fifo_t>
        static bool write1(fifo_t &fifo, const This *t) {
            return Impl::write1<sem>(fifo, element::forWriting(t));
        }
    };

    /** Indicates a sequence of protocol elements that should be written/read one after the other */
    template <typename... elements>
    struct Seq;

    template <typename... elements>
    struct Single<Seq<elements...>>: public Seq<elements...> {

    };

    template <typename e1, typename e2, typename... rest>
    struct Seq<e1, e2, rest...>{
        template <typename fifo_t>
        static ReadResult read1(fifo_t &fifo, This *t) {
            return Impl::readN(fifo, ReadResult::Valid, e1::forReading(t), e2::forReading(t), rest::forReading(t)...);
        }

        template <typename sem, typename fifo_t>
        static bool write1(fifo_t &fifo, const This *t) {
            return Impl::writeN<sem>(fifo, e1::forWriting(t), e2::forWriting(t), rest::forWriting(t)...);
        }
    };

    template <typename one>
    struct Seq<one>: public Single<one> {

    };

    /** Serializes a field as binary, little endian */
    template <typename int_t, int_t This::*field>
    struct Binary: public Single<Binary<int_t, field>> {
        static int_t* forReading(This *t) {
            return &(t->*field);
        }

        static int_t forWriting(const This *t) {
            return t->*field;
        }
    };

    /** Serializes a field as decimal */
    template <typename int_t, int_t This::*field>
    struct Decimal: public Single<Decimal<int_t, field>> {
        static ::Streams::Impl::Decimal<int_t*> forReading(This *t) {
            return ::Streams::Decimal(&(t->*field));
        }

        static ::Streams::Impl::Decimal<int_t> forWriting(const This *t) {
            return ::Streams::Decimal(t->*field);
        }
    };

    /** Serializes a field as hexadecimal */
    template <typename int_t, int_t This::*field>
    struct Hexadecimal: public Single<Hexadecimal<int_t, field>> {
        static ::Streams::Impl::Hexadecimal<int_t*> forReading(This *t) {
            return ::Streams::Hexadecimal(&(t->*field));
        }

        static ::Streams::Impl::Hexadecimal<int_t> forWriting(const This *t) {
            return ::Streams::Hexadecimal(t->*field);
        }
    };


    /** Serializes padding of indicated length bytes, written as 0, ignored during read */
    template <uint8_t length>
    struct Padding: public Single<Padding<length>> {
        static Streams::Padding forReading(This *t) {
            return length;
        }

        static Streams::Padding forWriting(const This *t) {
            return length;
        }
    };

    // In addition, you can also include a fixed String or ByteString token in a Protocol by saying
    // STR("foo") or BSTR(1,2,3). During writing the string will be written, during reading the string
    // will be verified.

    template <typename F, F This::*field, typename P = typename F::DefaultProtocol>
    struct Object: public Single<Object<F, field, P>> {
        static Impl::WithProtocol<F, P> forReading(This *t) {
            return &(t->*field);
        }
        static const Impl::WithProtocolConst<F, P> forWriting(const This *t) {
            return &(t->*field);
        }
    };

    template <bool (This::*condition)() const, typename... elements>
    struct Conditional: public Single<Conditional<condition, elements...>> {
        static auto forReading(This *t) {
            return ::Streams::Nested([t] (auto read) {
                return ((t->*condition)()) ? read(elements::forReading(t)...) : ReadResult::Valid;
            });
        }

        static auto forWriting(const This *t) {
            return ::Streams::Nested([t] (auto write) {
                return ((t->*condition)()) ? write(elements::forWriting(t)...) : true;
            });
        }
    };

    template <bool (This::*condition)() const, typename... elements>
    struct Conditional<condition, Seq<elements...>>: public Conditional<condition, elements...> {

    };

    template<typename ElementType, uint8_t count, ElementType (This::*field)[count]>
    struct Array: public Single<Array<ElementType, count, field>> {
        static ElementType (*forReading(This *t)) [count] {
            return &(t->*field);
        }

        static const ElementType (*forWriting(const This *t)) [count] {
            return &(t->*field);
        }

    };
};

/**
 * Returns a wrapper indicating to read/write [t] using the protocol given as template parameters.
 * This can be used in order to read/write a struct that does not have a DefaultProtocol typedef
 * (or customize a struct that does have one).
 */
template <typename... elements, typename T>
Impl::WithProtocol<T, typename Protocol<T>::template Seq<elements...>> as(T *t) {
    return t;
}

template <typename... elements, typename T>
Impl::WithProtocolConst<T, typename Protocol<T>::template Seq<elements...>> as(const T *t) {
    return t;
}

}

#endif /* STREAMS_PROTOCOL_HPP_ */
