#ifndef HAL_ATMEL_INTERRUPTVECTORS_HPP_
#define HAL_ATMEL_INTERRUPTVECTORS_HPP_

#include "gcc_type_traits.h"
#include "FOREACH.h"
#include <avr/interrupt.h>

namespace HAL {
namespace Atmel {

namespace InterruptVectors {


// source for hasVect: http://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
#define __mkHAS(vect) \
    template<typename, typename T> \
    struct has_##vect { \
        static_assert( \
            std::integral_constant<T, false>::value, \
            "Second template parameter needs to be of function type."); \
    }; \
\
    template<typename C, typename Ret, typename... Args> \
    struct has_##vect<C, Ret(Args...)> { \
    private: \
        template<typename T> \
        static constexpr auto check(T*) \
        -> typename \
            std::is_same< \
                decltype( std::declval<T>(). on##vect ( std::declval<Args>()... ) ), \
                Ret \
            >::type; \
\
        template<typename> \
        static constexpr std::false_type check(...); \
\
        typedef decltype(check<C>(0)) type; \
\
    public: \
        static constexpr bool value = type::value; \
    };

template<bool, typename _Tp, typename Fallback>
struct enable_ifelse
{
    typedef Fallback type;
};

template<typename _Tp, typename Fallback>
struct enable_ifelse<true, _Tp, Fallback>
{
    typedef _Tp type;
};

#define __mkINT_CALLBACK(vect) \
    struct has##vect {}; \
\
    template <typename target_t, target_t &target, typename check = void> struct vect##Callback {}; \
\
    template <typename target_t, target_t &target> \
    struct vect##Callback <target_t, target, typename std::enable_if<has_##vect<target_t, void()>::value>::type>: public has##vect { \
        static inline void on##vect () { \
            target.on##vect (); \
        } \
    };

#define __mkEXTEND_INT_CALLBACK(vect) \
    public vect##Callback<target_t, target>

#define __mkTYPEDEF_INT(vect) \
    typedef struct { \
        static inline void on##vect () {} \
    } vect##_Type; \

#define __mkTYPEDEF_IFELSE(vect) \
    typedef typename enable_ifelse<std::is_base_of<has##vect , vectorfor>::value, vectorfor, typename Next::vect##_Type>::type vect##_Type;

#define __mkTABLE_T(...) \
    FOR_EACH(__mkHAS, __VA_ARGS__) \
    FOR_EACH(__mkINT_CALLBACK, __VA_ARGS__) \
\
    template <typename target_t, target_t &target> \
    struct VectorCallback: \
        FOR_EACH_SEP_COMMA(__mkEXTEND_INT_CALLBACK, __VA_ARGS__) \
    {}; \
\
    template <typename... vectorsfor> \
    struct InterruptVectorTable { \
        FOR_EACH(__mkTYPEDEF_INT, __VA_ARGS__) \
    }; \
\
    template <typename vectorfor, typename... others> \
    struct InterruptVectorTable<vectorfor, others...> { \
        typedef InterruptVectorTable<others...> Next; \
\
        FOR_EACH(__mkTYPEDEF_IFELSE, __VA_ARGS__) \
    }; \



#define mkTABLE_T __mkTABLE_T(INT0_, USART_RX_, USART_UDRE_)

mkTABLE_T

#define __mkVECTOR_CALLBACK(var) \
    ::HAL::Atmel::InterruptVectors::VectorCallback<typeof var, var>

#define __mkISR(name) \
    ISR( name##vect ) { \
        __Table:: name##_Type :: on##name (); \
    }


/**
 * Generates the ISR bindings for any interrupt handlers that any of the given global variables
 * expose. Global variables expose interrupt handlers by declaring a public onXXX_ method, where
 * XXX is a handler name, e.g. "onINT0_()" for the INT0 vector.
 *
 * This variant declares at least an empty ISR for all interrupts supported by this library. If
 * you want more control, invoke ...... one by one, for the handlers you want.
 */
#define mkISRS(...) \
    typedef ::HAL::Atmel::InterruptVectors::InterruptVectorTable< \
        FOR_EACH_SEP_COMMA(__mkVECTOR_CALLBACK, __VA_ARGS__) \
    > __Table; \
\
    FOR_EACH(__mkISR, USART_RX_, INT0_, USART_UDRE_)

/**
 * Defines a method on a class that is an interrupt handler for the given vector [vect], e.g. INT0.
 * This can be a private method on the class, e.g.
 *
 *     class Pin {
 *         INTERRUPT_HANDLER(INT0) {
 *             // handle interrupt
 *         }
 *     }
 */
#define INTERRUPT_HANDLER(vect) \
    template <typename target_t, target_t &target> friend struct ::HAL::Atmel::InterruptVectors::vect##_Callback; \
    template <typename, typename> friend struct ::HAL::Atmel::InterruptVectors::has_##vect##_; \
    inline void on##vect##_()


}
}
}



#endif /* HAL_ATMEL_INTERRUPTVECTORS_HPP_ */
