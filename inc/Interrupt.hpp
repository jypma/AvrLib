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

class ScopedNoInterrupts {
    uint8_t const oldSREG;
public:
    ScopedNoInterrupts(): oldSREG(SREG) {
        cli();
    }
    ~ScopedNoInterrupts() {
        SREG = oldSREG;
    }
};

class InterruptHandler {
    volatile void (*func)(volatile void *) = nullptr;
    volatile void *ctx = nullptr;
protected:
    /** Invokes any registered handler */
    void invoke();
public:
    void attach(volatile void (*_func)(volatile void *), volatile void *_ctx);
    void attach(volatile void (*_func)(volatile void *));
    void detach();
};

#endif /* INTERRUPT_HPP_ */
