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

namespace Streams {

template<class Type>
class Streamable {

    template<typename CastedType, typename FieldType, FieldType Type::*field>
    class ScalarLiteral {
        static_assert(sizeof(CastedType) == sizeof(FieldType), "field type and casted type must have same size");
    public:
        static constexpr uint8_t minimumSize = sizeof(CastedType);
        static constexpr uint8_t maximumSize = minimumSize;

        template <typename fifo_t>
        inline static void __attribute__((optimize("unroll-loops"))) write(const Type &instance, fifo_t &fifo) {
            for (uint8_t i = 0; i < sizeof(CastedType); i++) {
                fifo.uncheckedWrite( ((uint8_t*)(&(instance.*field)))[i] );
            }
        }

        template <typename fifo_t>
        inline static void __attribute__((optimize("unroll-loops"))) read(fifo_t &fifo, Type &instance) {
            for (uint8_t i = 0; i < sizeof(CastedType); i++) {
                fifo.uncheckedRead( ((uint8_t*)(&(instance.*field)))[i] );
            }
        }
    };

public:

    template<typename FieldType, FieldType Type::*field, class Check = void>
    class Scalar {};

    template<bool Type::*field>
    class Scalar<bool, field>: public ScalarLiteral<uint8_t, bool, field> {};

    template<uint8_t Type::*field>
    class Scalar<uint8_t, field>: public ScalarLiteral<uint8_t, uint8_t, field> {};

    template<uint16_t Type::*field>
    class Scalar<uint16_t, field>: public ScalarLiteral<uint16_t, uint16_t, field> {};

    template<uint32_t Type::*field>
    class Scalar<uint32_t, field>: public ScalarLiteral<uint32_t, uint32_t, field> {};

    template<typename T, T Type::*field>
    class Scalar<T, field, typename std::enable_if<std::is_enum<T>::value>::type>: public ScalarLiteral<typename std::underlying_type<T>::type, T, field> {};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // uint16_t and up assumes little endian memory layout.
#else
#error Only little endian supported. Make sure to use gcc or define __BYTE_ORDER__ to be __ORDER_LITTLE_ENDIAN__
#endif


    template<typename ElementType, uint8_t count, ElementType (Type::*field)[count]>
    class Array {};

    template<uint8_t count, uint8_t (Type::*field)[count]>
    class Array<uint8_t, count, field> {
    public:
        static constexpr uint8_t minimumSize = count;
        static constexpr uint8_t maximumSize = minimumSize;

        template <typename fifo_t>
        inline static void __attribute__((optimize("unroll-loops"))) write(const Type &instance, fifo_t &fifo) {
            for (uint8_t i = 0; i < count; i++) {
                fifo.uncheckedWrite((instance.*field)[i]);
            }
        }

        template <typename fifo_t>
        inline static void __attribute__((optimize("unroll-loops"))) read(fifo_t &fifo, Type &instance) {
            for (uint8_t i = 0; i < count; i++) {
                fifo.uncheckedRead((instance.*field)[i]);
            }
        }
    };

    template<typename... Fields>
    class Format {
    public:
        // Fall-through for Fields is empty
        static constexpr uint8_t minimumSize = 0;
        static constexpr uint8_t maximumSize = 0;

        template <typename fifo_t>
        inline static void writeFields(const Type &instance, fifo_t &fifo) {}

        template <typename fifo_t>
        inline static void readFields(fifo_t &fifo, Type &instance) {}
    };

    template<typename Field, typename... Others>
    class Format<Field, Others...> {
    public:
        static constexpr uint8_t minimumSize = Field::minimumSize + Format<Others...>::minimumSize;
        static constexpr uint8_t maximumSize = Field::maximumSize + Format<Others...>::maximumSize;

        template <typename fifo_t>
        inline static void writeFields(const Type &instance, fifo_t &fifo) {
            Field::write(instance, fifo);
            Format<Others...>::writeFields(instance, fifo);
        }

        template <typename fifo_t>
        inline static void readFields(fifo_t &fifo, Type &instance) {
            Field::read(fifo, instance);
            Format<Others...>::readFields(fifo, instance);
        }
    };

    template<bool (Type::*condition)() const, typename... Fields>
    class Conditional: public Format<Fields...> {

    };

    template<bool (Type::*condition)() const, typename Field, typename... Others>
    class Conditional<condition, Field, Others...>: public Format<Field, Others...> {
        using Format<Field, Others...>::writeFields;
        using Format<Field, Others...>::readFields;
    public:
        static constexpr uint8_t minimumSize = 0;
        static constexpr uint8_t maximumSize = Field::maximumSize + Format<Others...>::maximumSize;

        template <typename fifo_t>
        inline static void write(const Type &instance, fifo_t &fifo) {
            if ((instance.*condition)()) {
                writeFields(instance, fifo);
            }
        }

        template <typename fifo_t>
        inline static void read(fifo_t &fifo, Type &instance) {
            if ((instance.*condition)()) {
                readFields(fifo, instance);
            }
        }
    };
};


}



#endif /* STREAMABLE_HPP_ */
