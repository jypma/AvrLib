/*
 * AtomicScope.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jan
 */

#ifndef ATOMICSCOPE_HPP_
#define ATOMICSCOPE_HPP_

#include <avr/common.h>
#include <avr/interrupt.h>

/**
 * Executes the scope in which the instance is declared atomically, i.e. with
 * the interrupt flag cleared, restoring the interrupt flag at the end of the scope
 * to its previous state.
 */
class AtomicScope {
    uint8_t const oldSREG;
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

    inline __attribute__((always_inline)) AtomicScope(): oldSREG(SREG) {
        cli();
    }
    inline __attribute__((always_inline)) ~AtomicScope() {
        SREG = oldSREG;
    }
};

#endif /* ATOMICSCOPE_HPP_ */
