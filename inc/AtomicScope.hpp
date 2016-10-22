#pragma once

#include <HAL/Atmel/Registers.hpp>

#ifdef AVR
# define sei()  __asm__ __volatile__ ("sei" ::: "memory")
# define cli()  __asm__ __volatile__ ("cli" ::: "memory")
#else
# define sei() { HAL::Atmel::Registers::SREG_I.set(); }
# define cli() { HAL::Atmel::Registers::SREG_I.clear(); }
#endif

/**
 * Executes the scope in which the instance is declared atomically, i.e. with
 * the interrupt flag cleared, restoring the interrupt flag at the end of the scope
 * to its previous state.
 */
class AtomicScope {
    HAL::Atmel::Registers::SREG_t const oldSREG;
public:
    /**
     * Executes the scope in which the instance is declared atomically, i.e. with
     * the interrupt flag cleared, enabling the interrupt flag at the end of the scope
     * (regardless what the interrupt flag was beforehand).
     */
    class SEI {
    public:
        inline __attribute__((always_inline)) SEI() {
            cli();
        }
        inline __attribute__((always_inline)) ~SEI() {
            sei();
        }
    };

    inline __attribute__((always_inline)) AtomicScope(): oldSREG(HAL::Atmel::Registers::SREG) {
        cli();
    }
    inline __attribute__((always_inline)) ~AtomicScope() {
    	HAL::Atmel::Registers::SREG = oldSREG;
    }
};

