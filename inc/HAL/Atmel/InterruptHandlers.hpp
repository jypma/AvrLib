#ifndef HAL_ATMEL_INTERRUPTHANDLERS_HPP_
#define HAL_ATMEL_INTERRUPTHANDLERS_HPP_

#include "gcc_type_traits.h"
#include "FOREACH.h"
#include <avr/interrupt.h>

namespace HAL {
namespace Atmel {

namespace Impl {

template <typename I>
struct HardwareInt {
    typedef I INT;

    template <typename body_t>
    static __attribute__((always_inline)) inline void wrap(body_t body) {
        body();
    }
};

template <typename T>
struct EmptyHandlers {
    template <typename V>
    struct Handler {
        static_assert(sizeof(typename V::INT) == sizeof(typename V::INT), "T in Handler<T> must have be an interrupt vector type.");

        static __attribute__((always_inline)) inline void invoke(T &t) {}
    };
};

template <typename I, typename T, void (T::*f)(), typename P, typename V, typename check=void>
struct On_Handler {
    static __attribute__((always_inline)) inline void invoke(T &t) {
        P::template Handler<V>::invoke(t);
    }
};

template <typename I, typename T, void (T::*f)(), typename P, typename V>
struct On_Handler<I,T,f,P,V,typename std::enable_if<std::is_same<V, typename I::INT>::value>::type> {
    static __attribute__((always_inline)) inline void invoke(T &t) {
        T * const t_ptr = &t;
        I::wrap([t_ptr] () __attribute__((always_inline)) {
            (t_ptr->*f)();
        });
        P::template Handler<V>::invoke(t);
    }
};

}

/**
 * Interrupt handlers are defined by declaring a public typedef named "Handlers" inside your class.
 * "Handlers" should be an alias to chained "On" and "Delegate" types.
 */
namespace InterruptHandlers {
    /**
     * Defines a single interrupt handler.
     * @param T This-type of the class defining the interrupt handler
     * @param I Interrupt vector to bind to. One of Int_*, or a custom vector.
     * @param f The method to call. Must be void and no-args, but can be private.
     * @param P next handler in the chain, can be another "On" or "Delegate".
     */
    template <typename T, typename I, void (T::*f)(), typename P = Impl::EmptyHandlers<T>>
    struct On: public P {
        template <typename V>
        using Handler = Impl::On_Handler<I,T,f,P,V>;
    };

    /**
     * Defines delegating of interrupt handlers down to a member of this class. The member
     * must have a public typedef called "Handlers", itself being an alias to chained "On" and
     * "Delegate" types.
     * @param T This-type of the class defining the interrupt handler
     * @param U Type of the field to delegate to
     * @param u The actual field on T to delegate to
     * @param P next handler in the chain, can be another "On" or "Delegate".
     */
    template <typename T, typename U, U T::*u, typename P = Impl::EmptyHandlers<T>>
    struct Delegate: public P {
        template <typename I>
        struct Handler {
            static __attribute__((always_inline)) inline void invoke(T &t) {
                U::Handlers::template Handler<I>::invoke(t.*u);
                P::template Handler<I>::invoke(t);
            }
        };
    };
};


#define mkINT(name) struct Int_##name : public Impl::HardwareInt< Int_##name > {};

#define mkINTS(...) \
    FOR_EACH(mkINT, __VA_ARGS__)

mkINTS(
        WDT_, \
        ADC_, \
        TWI_, \
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

#define mkISR(name) \
    ISR( name##vect ) { \
        decltype(app)::Handlers::Handler < ::HAL::Atmel::Int_##name >::invoke(app); \
    }

/**
 * Declares that the main app running on the chip is of the given type. The type must be a class or struct, having
 * the following public members:
 *
 * - A typedef "Handlers" aliasing an {@link InterruptHandlers::On} or {@link InterruptHandlers::Delegate} type,
 *   to define interrupt handlers.
 * - A void method main(), which will become the main function.
 */
#define RUN_APP(type) \
    type app; \
    int main() { app.main(); } \
    mkISRS \

}
}

#endif /* HAL_ATMEL_INTERRUPTHANDLERS_HPP_ */
