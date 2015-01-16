/*
 * Interrupt.hpp
 *
 *  Created on: Dec 26, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_HPP_
#define INTERRUPT_HPP_

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
        SEI() {
            cli();
        }
        ~SEI() {
            sei();
        }
    };

    AtomicScope(): oldSREG(SREG) {
        cli();
    }
    ~AtomicScope() {
        SREG = oldSREG;
    }
};

class InterruptHandler {
protected:
    /** Invokes any registered handler */
    void invoke();
public:
    void (*func)(volatile void *) = nullptr;
    volatile void *ctx = nullptr;
    void attach(void (*_func)(volatile void *), volatile void *_ctx);
    void attach(void (*_func)(volatile void *));
    void detach();
};

#endif /* INTERRUPT_HPP_ */
