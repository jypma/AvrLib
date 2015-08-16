/*
 * Streamable.hpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jan
 */

#ifndef STREAMABLE_HPP_
#define STREAMABLE_HPP_

#include "gcc_type_traits.h"
#include <stdint.h>
#include "Streams/Format.hpp"
#include "ChunkedFifo.hpp"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif

namespace Streams {

    template<char... tokenChars>
    class Token {
    public:
        // Fall-through for tokenChars is empty
        static constexpr uint8_t minimumSize = 0;
        static constexpr uint8_t maximumUncheckedWriteSize = 0;

        template <typename writer_t>
        inline static void writeField(writer_t &out) {}

        template <typename reader_t>
        inline static void readField(reader_t &in) {}
    };

    template<char ch, char...others>
    class Token<ch, others...> {
    public:
        static constexpr uint8_t minimumSize = 1 + Token<others...>::minimumSize;
        static constexpr uint8_t maximumUncheckedWriteSize = 1 + Token<others...>::maximumUncheckedWriteSize;

        template <typename writer_t>
        inline static void writeField(writer_t &out) {
            out.uncheckedWrite(uint8_t(ch));
            Token<others...>::writeFields(out);
        }

        template <typename reader_t, typename T>
        inline static void readField(reader_t &in, T &t) {
            readField(in);
        }

        template <typename reader_t>
        inline static void readField(reader_t &in) {
            uint8_t input;
            in.uncheckedRead(input);
            if (input != ch) {
                in.markInvalid();
            } else {
                Token<others...>::readField(in);
            }
        }
    };

    template<char ch1, char ch2, char...others>
    class Token<ch1, ch2, others...> {
    public:
        static constexpr uint8_t minimumSize = 1;
        static constexpr uint8_t maximumUncheckedWriteSize = 1 + Token<others...>::maximumUncheckedWriteSize;

        template <typename writer_t>
        inline static void writeField(writer_t &out) {
            out.uncheckedWrite(uint8_t(ch1));
            Token<ch2,others...>::writeFields(out);
        }

        template <typename reader_t, typename T>
        inline static void readField(reader_t &in, T &t) {
            readField(in);
        }

        template <typename reader_t>
        inline static void readField(reader_t &in) {
            uint8_t input;
            in.uncheckedRead(input);
            if (input != ch1) {
                in.markInvalid();
            } else {
                if (in.getReadAvailable() >= Token<ch2,others...>::minimumSize) {
                    Token<ch2,others...>::readField(in);
                } else {
                    in.markPartial();
                }
            }
        }
    };

namespace Parts {

    template<typename Type, typename CastedType, typename FieldType, FieldType Type::*field>
    class ScalarLiteral {
        static_assert(sizeof(CastedType) == sizeof(FieldType), "field type and casted type must have same size");
    public:
        static constexpr uint8_t minimumSize = sizeof(CastedType);
        static constexpr uint8_t maximumUncheckedWriteSize = minimumSize;

        template <typename writer_t>
        inline static void __attribute__((optimize("unroll-loops"))) writeField(const Type &instance, writer_t &out) {
            for (uint8_t i = 0; i < sizeof(CastedType); i++) {
                out.uncheckedWrite( ((uint8_t*)(&(instance.*field)))[i] );
            }
        }

        template <typename reader_t>
        inline static void __attribute__((optimize("unroll-loops"))) readField(reader_t &in, Type &instance) {
            for (uint8_t i = 0; i < sizeof(CastedType); i++) {
                in.uncheckedRead( ((uint8_t*)(&(instance.*field)))[i] );
            }
        }
    };

    template<typename Type, typename FieldType, FieldType Type::*field, class Check = void>
    class Scalar {};

    template<typename Type, bool Type::*field>
    class Scalar<Type, bool, field>: public ScalarLiteral<Type, uint8_t, bool, field> {};

    template<typename Type, uint8_t Type::*field>
    class Scalar<Type, uint8_t, field>: public ScalarLiteral<Type, uint8_t, uint8_t, field> {};

    template<typename Type, uint16_t Type::*field>
    class Scalar<Type, uint16_t, field>: public ScalarLiteral<Type, uint16_t, uint16_t, field> {};

    template<typename Type, uint32_t Type::*field>
    class Scalar<Type, uint32_t, field>: public ScalarLiteral<Type, uint32_t, uint32_t, field> {};

    template<typename Type, typename T, T Type::*field>
    class Scalar<Type, T, field, typename std::enable_if<std::is_enum<T>::value>::type>: public ScalarLiteral<Type, typename std::underlying_type<T>::type, T, field> {};

    template<typename Type, typename ElementType, uint8_t count, ElementType (Type::*field)[count]>
    class Array {};

    template<typename Type, uint8_t count, uint8_t (Type::*field)[count]>
    class Array<Type, uint8_t, count, field> {
    public:
        static constexpr uint8_t minimumSize = count;
        static constexpr uint8_t maximumUncheckedWriteSize = minimumSize;

        template <typename writer_t>
        inline static void __attribute__((optimize("unroll-loops"))) writeField(const Type &instance, writer_t &out) {
            for (uint8_t i = 0; i < count; i++) {
                out.uncheckedWrite((instance.*field)[i]);
            }
        }

        template <typename reader_t>
        inline static void __attribute__((optimize("unroll-loops"))) readField(reader_t &in, Type &instance) {
            for (uint8_t i = 0; i < count; i++) {
                in.uncheckedRead((instance.*field)[i]);
            }
        }
    };

    template<typename Type, typename... Fields>
    class Format;

    /**
     * Reads or writes a chunk from/to a ChunkedFifo, prepended by writing
     * the length in decimal, and an optional token in between the length and the data.
     */
    template<typename Type, ChunkedFifo Type::*field, typename Separator = Format<Type>>
    struct Chunk {
        static constexpr uint8_t minimumSize = 1 + Separator::minimumSize;
        static constexpr uint8_t maximumUncheckedWriteSize = 0 + Separator::maximumUncheckedWriteSize;

        static bool isDigit(uint8_t digit) {
            return digit >= '0' && digit <= '9';
        }

        template <typename reader_t>
        inline static void readField(reader_t &in, Type &instance) {
            uint8_t length;
            uint8_t digit;

            in.uncheckedRead(digit); // digit 0
            if (isDigit(digit)) {
                if (digit != '0') {
                    length = digit - '0';
                    uint8_t available = in.getReadAvailable();
                    if (available > 0) {
                        if (isDigit(in.peek())) {
                            in.uncheckedRead(digit); // digit 1
                            length = length * 10 + (digit - '0');
                            available--;
                            if (isDigit(in.peek())) {
                                in.uncheckedRead(digit); // digit 2
                                if ((length <= 24) || (length == 25 && digit <= '5')) {
                                    length = length * 10 + (digit - '0');
                                } else {
                                    // Overflow for uint8_t, that's not OK.
                                    in.markInvalid();
                                    return;
                                }
                            } else {
                                // Non-digit as third char, that's OK, let's see if the separator matches.
                            }
                        } else {
                            // Non-digit as second char, that's OK, let's see if the separator matches.
                        }
                    } else {
                        // Got first digit >0, so there should at least be another byte with more digit, or actual data.
                        in.markIncomplete();
                        return;
                    }
                } else {
                    // First digit was '0', so length was zero.
                }
            } else {
                // Expecting a digit as first char, didn't get one
                in.markInvalid();
                return;
            }

            // We got the length now. Reading separator.
            Separator::readFields(in, instance);

            if (in) {
                // Separator is OK, read the chunk.
                uint8_t available = in.getReadAvailable();
                if (available >= length) {
                    if ((instance.*field).getSpace() >= length) {
                        (instance.*field).writeStart();
                        while (length > 0) {
                            uint8_t b;
                            in.uncheckedRead(b);
                            (instance.*field).uncheckedWrite(b);
                            length--;
                        }
                        (instance.*field).writeEnd();
                    } else {
                        // No space, just drop the chunk.
                        while (length > 0) {
                            uint8_t b;
                            in.uncheckedRead(b);
                            length--;
                        }
                    }
                } else {
                    // Not enough bytes yet for the whole chunk.
                    in.markIncomplete();
                    return;
                }
            }
        }

    };

    template<typename Type, typename... Fields>
    class Format {
    public:
        // Fall-through for Fields is empty
        static constexpr uint8_t minimumSize = 0;
        static constexpr uint8_t maximumUncheckedWriteSize = 0;

        template <typename writer_t>
        inline static void writeFields(const Type &instance, writer_t &out) {}

        template <typename reader_t>
        inline static void readFields(reader_t &in, Type &instance) {}

        template <typename reader_t>
        inline static void readFields(reader_t &in) {}
    };

    template<typename Type, typename Field, typename... Others>
    class Format<Type, Field, Others...> {
    public:
        static constexpr uint8_t minimumSize = Field::minimumSize + Format<Type, Others...>::minimumSize;
        static constexpr uint8_t maximumUncheckedWriteSize = Field::maximumUncheckedWriteSize + Format<Type, Others...>::maximumUncheckedWriteSize;

        template <typename writer_t>
        inline static void writeFields(const Type &instance, writer_t &out) {
            Field::writeField(instance, out);
            Format<Type, Others...>::writeFields(instance, out);
        }

        template <typename reader_t>
        inline static void readFields(reader_t &in, Type &instance) {
            Field::readField(in, instance);
            Format<Type, Others...>::readFields(in, instance);
        }

        template <typename reader_t>
        inline static void readFields(reader_t &in) {
            Field::readField(in);
            Format<Type, Others...>::readFields(in);
        }

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            if (in.getReadAvailable() >= minimumSize) {
                readFields(in, instance);
            } else {
                in.markIncomplete();
            }
        }

        template <typename reader_t>
        inline static void read(reader_t &in) {
            if (in.getReadAvailable() >= minimumSize) {
                readFields(in);
            } else {
                in.markIncomplete();
            }
        }
    };

    template<typename Type, bool (Type::*condition)() const, typename... Fields>
    class Conditional: public Format<Type, Fields...> {};

    template<typename Type, bool (Type::*condition)() const, typename Field, typename... Others>
    class Conditional<Type, condition, Field, Others...>: public Format<Type, Field, Others...> {
        using Format<Type, Field, Others...>::writeFields;
        using Format<Type, Field, Others...>::readFields;
    public:
        static constexpr uint8_t minimumSize = 0;
        static constexpr uint8_t minimumSizeWhenPresent = Format<Type, Field, Others...>::minimumSize;
        static constexpr uint8_t maximumUncheckedWriteSize = Format<Type, Field, Others...>::maximumUncheckedWriteSize;

        template <typename writer_t>
        inline static void writeField(const Type &instance, writer_t &out) {
            if ((instance.*condition)()) {
                writeFields(instance, out);
            }
        }

        template <typename reader_t>
        inline static void readField(reader_t &in, Type &instance) {
            if ((instance.*condition)()) {
                if (in.getReadAvailable() >= minimumSizeWhenPresent) {
                    readFields(in, instance);
                } else {
                    in.markIncomplete();
                }
            }
        }
    };

} // end namespace Parts

template<class Type>
struct Streamable {
    template <typename... Fields> using Format = Parts::Format<Type, Fields...>;
    template <typename FieldType, FieldType Type::*field, class Check = void> using Scalar = Parts::Scalar<Type, FieldType, field, Check>;
    template <bool (Type::*condition)() const, typename... Fields> using Conditional = Parts::Conditional<Type, condition, Fields...>;
    template <typename ElementType, uint8_t count, ElementType (Type::*field)[count]> using Array = Parts::Array<Type, ElementType, count, field>;
    template <ChunkedFifo Type::*field, typename Separator = Format<Type>> using Chunk = Parts::Chunk<Type, field, Separator>;

};

struct NoType {};

template <typename... Fields>
class Seq: public Parts::Format<NoType, Fields...> {};


} // end namespace Streams



#endif /* STREAMABLE_HPP_ */
