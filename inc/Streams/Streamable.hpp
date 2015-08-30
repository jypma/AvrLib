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
#include "Logging.hpp"


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif

namespace Streams {

    template <typename reader_t>
    void Token__readFromFlash(reader_t &in, const char *addr, uint8_t length) {
        uint8_t available = in.getReadAvailable();
        if (available == 0) {
            in.markIncomplete();
        }
        uint8_t first;
        in.uncheckedRead(first);
        available--;

        if (first != pgm_read_byte(addr)) {
            in.markInvalid();
            return;
        }

        for (uint8_t i = 1; i < length; i++) {
            if (available == 0) {
                in.markPartial();
                return;
            }
            uint8_t ch;
            in.uncheckedRead(ch);
            available--;

            addr++;
            if (ch != pgm_read_byte(addr)) {
                in.markInvalid();
                return;
            }
        }
    }

    template <typename writer_t>
    void Token__writeFromFlash(writer_t &out, const char *addr, uint8_t length) {
        if (out.hasSpace(length)) {
            for (uint8_t i = 0; i < length; i++) {
                out.writeUnchecked(addr);
                addr++;
            }
        } else {
            out.markInvalid();
        }
    }

    /**
     * Represents a fixed set of characters in a format.
     * @param str A typestring, returned by the STR macro in Strings.hpp
     */
    template<typename str>
    struct Token {
        typedef Logging::Log<Loggers::Streams> log;

        static constexpr uint8_t fixedSize = 0; // disable the outer fixed size optimization, since we do want to read the first token when scanning.
        static_assert(str::size() > 0, "Token must have non-empty STR as template argument.");

        template <typename writer_t>
        static void write (writer_t &out) {
            Token__writeFromFlash(out, str::data(), str::size());
        }

        template <typename reader_t, typename Type>
        inline static void read (reader_t &in, Type &instance) {
            read(in);
        }

        template <typename reader_t>
        static void read (reader_t &in) {
            Token__readFromFlash(in, str::data(), str::size());
        }
    };

namespace Parts {

    template<typename Type, typename CastedType, typename FieldType, FieldType Type::*field>
    class ScalarLiteral {
        static_assert(sizeof(CastedType) == sizeof(FieldType), "field type and casted type must have same size");
    public:
        static constexpr uint8_t fixedSize = sizeof(CastedType);

        template <typename writer_t>
        inline static void __attribute__((optimize("unroll-loops"))) writeUnchecked(writer_t &out, const Type &instance) {
            for (uint8_t i = 0; i < sizeof(CastedType); i++) {
                out.uncheckedWrite( ((uint8_t*)(&(instance.*field)))[i] );
            }
        }

        template <typename reader_t>
        inline static void __attribute__((optimize("unroll-loops"))) readUnchecked(reader_t &in, Type &instance) {
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
        static constexpr uint8_t fixedSize = count;

        template <typename writer_t>
        inline static void __attribute__((optimize("unroll-loops"))) writeUnchecked(writer_t &out, const Type &instance) {
            for (uint8_t i = 0; i < count; i++) {
                out.uncheckedWrite((instance.*field)[i]);
            }
        }

        template <typename reader_t>
        inline static void __attribute__((optimize("unroll-loops"))) readUnchecked(reader_t &in, Type &instance) {
            for (uint8_t i = 0; i < count; i++) {
                in.uncheckedRead((instance.*field)[i]);
            }
        }
    };
/*
    template<typename Type, typename... Fields>
    class Format;
*/
    /**
     * Reads or writes a chunk from/to a ChunkedFifo, prepended by writing
     * the length in decimal, and an optional token in between the length and the data.
     */
    template<typename Type, ChunkedFifo Type::*field, typename Separator/* = Format<Type>*/>
    struct Chunk {
        static constexpr uint8_t fixedSize = 0;        // Chunk is not fixed size

        static bool isDigit(uint8_t digit) {
            return digit >= '0' && digit <= '9';
        }

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            uint8_t length = 0;
            uint8_t digit;

            if (in.getReadAvailable() < 1 + Separator::fixedSize) {
                in.markIncomplete();
                return;
            }

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
            Separator::read(in, instance);

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

    template <typename Field> struct IsFixedSize {
        static constexpr bool value = Field::fixedSize > 0;
    };

    template<typename Type, typename check, typename... Fields>
    struct F {
        // fallback for empty
        static constexpr uint8_t fixedSize = 0;

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {}

        template <typename reader_t>
        inline static void read(reader_t &in) {}

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in, Type &instance) {}

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in) {}

        template <typename writer_t>
        inline static void write(writer_t &out, const Type &instance) {}

        template <typename writer_t>
        inline static void writeUnchecked(writer_t &out, const Type &instance) {}
    };

    template<typename Type, typename... Fields>
    using Format = F<Type, void, Fields...>;

    template<typename Type, typename Field, typename... Fields>
    struct F<Type, typename std::enable_if<IsFixedSize<Field>::value>::type, Field, Fields...> {
        typedef Logging::Log<Loggers::Streams> log;
        static constexpr uint8_t fixedSize = Field::fixedSize + Format<Type,Fields...>::fixedSize;
        // first field is a fixed size

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            if (in.getReadAvailable() >= fixedSize) {
                Field::readUnchecked(in, instance);
                Format<Type,Fields...>::readUnchecked(in, instance);
            } else {
                in.markIncomplete();
            }
        }

        template <typename reader_t>
        inline static void read(reader_t &in) {
            if (in.getReadAvailable() >= fixedSize) {
                Field::readUnchecked(in);
                Format<Type,Fields...>::readUnchecked(in);
            } else {
                in.markIncomplete();
            }
        }

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in, Type &instance) {
            // We reached here, so the second field also was fixedSize, and has already been checked.
            Field::readUnchecked(in, instance);
            Format<Type,Fields...>::readUnchecked(in, instance);
        }

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in) {
            // We reached here, so the second field also was fixedSize, and has already been checked.
            Field::readUnchecked(in);
            Format<Type,Fields...>::readUnchecked(in);
        }

        template <typename writer_t>
        inline static void write(writer_t &out, const Type &instance) {
            log::debug("Attemting to output %d bytes\n", fixedSize);
            if (out.hasSpace(fixedSize)) {
                log::debug("  OK.\n");
                Field::writeUnchecked(out, instance);
                Format<Type,Fields...>::writeUnchecked(out, instance);
            } else {
                out.markInvalid();
            }
        }

        template <typename writer_t>
        inline static void writeUnchecked(writer_t &out, const Type &instance) {
            // We reached here, so the second field also was fixedSize, and has already been checked.
            Field::writeUnchecked(out, instance);
            Format<Type,Fields...>::writeUnchecked(out, instance);
        }
    };

    template<typename Type, typename Field, typename... Fields>
    struct F<Type, typename std::enable_if<!IsFixedSize<Field>::value>::type, Field, Fields...> {
        // first field is not a fixed size
        static constexpr uint8_t fixedSize = 0;

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in, Type &instance) {
            // No more unchecked read possible, since we've crossed into a non-fixed size field.
            Field::read(in, instance);

            if (in) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::read(in, instance);
            }
        }

        template <typename reader_t>
        inline static void readUnchecked(reader_t &in) {
            // No more unchecked read possible, since we've crossed into a non-fixed size field.
            Field::read(in);

            if (in) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::read(in);
            }
        }

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            // Format starts with a non-fixed size field, so we always read checked.
            Field::read(in, instance);

            if (in) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::read(in, instance);
            }
        }

        template <typename reader_t>
        inline static void read(reader_t &in) {
            // Format starts with a non-fixed size field, so we always read checked.
            Field::read(in);

            if (in) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::read(in);
            }
        }

        template <typename writer_t>
        inline static void writeUnchecked(writer_t &out, const Type &instance) {
            // No more unchecked write possible, since we've crossed into a non-fixed size field.
            Field::write(out, instance);

            if (out) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::write(out, instance);
            }
        }

        template <typename writer_t>
        inline static void write(writer_t &out, Type &instance) {
            // Format starts with a non-fixed size field, so we always write checked.
            Field::write(out, instance);

            if (out) {
                // Treat the next field(s) possibly as fixed again.
                Format<Type,Fields...>::write(out, instance);
            }
        }


    };
/*
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
        typedef Logging::Log<Loggers::Streams> log;
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
            if (in) {
                if (in.getReadAvailable() >= minimumSize) {
                    log::debug("  reading a field, available %d, minimum size %d, state %d\n", in.getReadAvailable(), minimumSize, (ReaderState) in);
                    Field::readField(in, instance);
                    Format<Type, Others...>::readFields(in, instance);
                } else {
                    in.markIncomplete();
                }
            }
        }

        template <typename reader_t>
        inline static void readFields(reader_t &in) {
            if (in) {
                if (in.getReadAvailable() >= minimumSize) {
                    Field::readField(in);
                    Format<Type, Others...>::readFields(in);
                } else {
                    in.markIncomplete();
                }
            }
        }

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            log::debug("Starting to read, available %d, minimum size %d, state %d\n", in.getReadAvailable(), minimumSize, (ReaderState) in);
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
*/
    template<typename Type, bool (Type::*condition)() const, typename... Fields>
    struct Conditional {
        typedef Format<Type,Fields...> inner;
        static constexpr uint8_t fixedSize = 0; // Conditional doesn't have a fixed size, because it conditionally appears.

        template <typename reader_t>
        inline static void read(reader_t &in, Type &instance) {
            if ((instance.*condition)()) {
                inner::read(in, instance);
            }
        }

        template <typename writer_t>
        inline static void write(writer_t &out, const Type &instance) {
            if ((instance.*condition)()) {
                inner::write(out, instance);
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
