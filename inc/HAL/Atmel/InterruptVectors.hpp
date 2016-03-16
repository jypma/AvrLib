#ifndef HAL_ATMEL_INTERRUPTVECTORS_HPP_
#define HAL_ATMEL_INTERRUPTVECTORS_HPP_

#include "gcc_type_traits.h"
#include "FOREACH.h"
#include <avr/interrupt.h>

namespace HAL {
namespace Atmel {
namespace InterruptVectors {


template<bool, typename _Tp, typename Fallback = void>
struct enable_ifelse
{
    typedef Fallback type;
};

template<typename _Tp, typename Fallback>
struct enable_ifelse<true, _Tp, Fallback>
{
    typedef _Tp type;
};

template <typename VectorName, typename T, void (T::*f)(), typename check = void>
class Handler {
    static constexpr void (T::*function)() = f;
public:
    typedef T Type;
    typedef VectorName VECT;
    static void invoke(T * const t) {
        (t->*f)();
    }
};

/**
 * Handler definition for where the first template argument is in fact a wrapping vector.
 * A wrapping vector must define a nested type VECT, which is the actual interrupt vector,
 * and a static wrapping function
 *
 *     template <typename body_t>
 *     static void wrap(body_t body) {
 *          // possibly invoke body() at some point
 *     }
 *
 * The wrapper is invoked whenever the interrupt occurs, and can choose to delegate to the
 * actual class that is listening on this handler.
 */
template <typename VectorName, typename T, void (T::*f)()>
class Handler<VectorName, T, f, typename std::enable_if<std::is_class<typename VectorName::VECT>::value>::type> {
    static constexpr void (T::*function)() = f;
public:
    typedef T Type;
    typedef typename VectorName::VECT VECT;
    static void invoke(T * const t) {
        VectorName::wrap([t] {
            (t->*f)();
        });
    }
};


template <typename handler, typename handler::Type &t, typename check = void>
struct Callback {};

#define mkVECTOR(vect) \
    struct Vector##vect {} ; \
\
    template <typename handler, typename handler::Type &t> \
    struct Callback<handler, t, typename std::enable_if<std::is_same<typename handler::VECT, Vector##vect >::value>::type>: public Vector##vect { \
        static inline void on##vect () { \
            handler::invoke(&t); \
        } \
    }; \
\
    template <typename callback1, typename callback2> \
    struct Chain##vect { \
        static inline void on##vect () { \
            callback1::on##vect (); \
            callback2::on##vect (); \
        } \
    };


template <typename type, type &t, typename check = void>
struct Callbacks2 {};

template <typename type, type &t>
struct Callbacks2<type, t, typename enable_ifelse<false, typename type::Handler2>::type>: public Callback<typename type::Handler2, t>{};

template <typename type, type &t, typename check = void>
struct Callbacks1 {};

template <typename type, type &t>
struct Callbacks1<type, t, typename enable_ifelse<false, typename type::Handler1>::type> : public Callback<typename type::Handler1, t>, public Callbacks2<type, t> {};

#define __mkTYPEDEF_INT(vect) \
    typedef struct { \
        static inline void on##vect () {} \
    } vect##Type; \

#define __mkTYPEDEF_IFELSE(vect) \
    typedef typename enable_ifelse<std::is_base_of<Vector##vect , callback>::value, Chain##vect<callback, typename Next::vect##Type>, typename Next::vect##Type>::type vect##Type;

#define mkVECTORS(...) \
    FOR_EACH(mkVECTOR, __VA_ARGS__) \
\
    template <typename... callbacks> \
    struct InterruptVectorTable { \
        FOR_EACH(__mkTYPEDEF_INT, __VA_ARGS__) \
    }; \
\
    template <typename callback, typename... others> \
    struct InterruptVectorTable<callback, others...> { \
        typedef InterruptVectorTable<others...> Next; \
\
        FOR_EACH(__mkTYPEDEF_IFELSE, __VA_ARGS__) \
    }; \

/**
 * Defines all types for the complete list of ISR names that might exist on ANY Atmel chip.
 */
mkVECTORS(\
        WDT_, \
        ADC_, \
        INT0_, \
        INT1_, \
        TIMER0_OVF_, \
        TIMER0_COMPA_, \
        TIMER0_COMPB_, \
        TIMER1_OVF_, \
        TIMER1_COMPA_, \
        TIMER1_COMPB_, \
        TIMER2_OVF_, \
        TIMER2_COMPA_, \
        TIMER2_COMPB_, \
        USART_RX_, \
        USART_UDRE_, \
        PCINT0_, \
        PCINT1_, \
        PCINT2_)

#define __mkVECTOR_CALLBACK(var) \
    ::HAL::Atmel::InterruptVectors::Callbacks1<decltype(var), var>

#define __mkISR(name) \
    ISR( name##vect ) { \
        __Table:: name##Type :: on##name (); \
    }

/**
 * Generates the ISR bindings for any interrupt handlers that any of the given global variables
 * expose. Global variables expose interrupt handlers by calling the INTERRUPT_HANDLER1 macro in
 * its class definition.
 *
 * Bindings for ALL interrupt vectors known to this library are generated, even when not used
 * (they'll be empty functions then). This is a limitation of the C++ / macro auto-detection.
 * It shouldn't generally be a problem, since the specific interrupts still have to be enabled
 * for there to be a runtime overhead. It only uses a few bytes of extra flash.
 */
#define mkISRS(...) \
    typedef ::HAL::Atmel::InterruptVectors::InterruptVectorTable< \
        FOR_EACH_SEP_COMMA(__mkVECTOR_CALLBACK, __VA_ARGS__) \
    > __Table; \
\
    __mk_ALL_ISRS

/**
 * Declares an interrupt handler in a class definition. The method name must be an instance method
 * of the class, and can be private. This macro must appear LAST in the class definition.
 * The enclosing type must define have a This typedef, pointing at itself.
 *
 * @param vect The interrupt vector, e.g. `typename pin_t::INT` or an exact vector by
 *             invoking `INTERRUPT_VECTOR(USART_UDRE)`
 * @param method The method to invoke.
 */
#define INTERRUPT_HANDLER1(vect, method) \
    template <typename, typename, typename> friend struct ::HAL::Atmel::InterruptVectors::Callback; \
public: \
    typedef ::HAL::Atmel::InterruptVectors::Handler<vect, This, &This::method> Handler1; \
    typedef ::HAL::Atmel::InterruptVectors::Handler<vect, This, &This::method> method##Handler;

/**
 * Declares an interrupt handler in a class definition. The method name must be an instance method
 * of the class, and can be private. This macro must appear LAST in the class definition.
 * The enclosing type must define have a This typedef, pointing at itself.
 *
 * @param vect The interrupt vector, e.g. `typename pin_t::INT` or an exact vector by
 *             invoking `INTERRUPT_VECTOR(USART_UDRE)`
 * @param method The method to invoke.
 */
#define INTERRUPT_HANDLER2(vect, method) \
    template <typename, typename, typename> friend struct ::HAL::Atmel::InterruptVectors::Callback; \
public: \
    typedef ::HAL::Atmel::InterruptVectors::Handler<vect, This, &This::method> Handler2; \
    typedef ::HAL::Atmel::InterruptVectors::Handler<vect, This, &This::method> method##Handler;


#define INTERRUPT_VECTOR(vect) ::HAL::Atmel::InterruptVectors::Vector##vect##_

}
}
}



#endif /* HAL_ATMEL_INTERRUPTVECTORS_HPP_ */
