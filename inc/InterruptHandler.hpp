/*
 * Interrupt.hpp
 *
 *  Created on: Dec 26, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_HPP_
#define INTERRUPT_HPP_

class InterruptHandler {
    void (*func)(volatile void *) = nullptr;
    volatile void *ctx = nullptr;
public:
    /**
     * Attaches the given function and context to this InterruptHandler.
     *
     * Expects the function to invoke other interrupt handlers for the same interrupt, by
     * calling .invoke() on the return value to this function.
     */
    InterruptHandler attach(void (*_func)(volatile void *), volatile void *_ctx);

    /**
     * Attaches the given function to this InterruptHandler (it will get null as ctx).
     * Expects the function to invoke other interrupt handlers for the same interrupt, by
     *
     * calling .invoke() on the return value to this function.
     */
    InterruptHandler attach(void (*_func)(volatile void *));

    /** Invokes this handler, and any earlier handlers encountered in its registration. */
    void invoke();
};

#endif /* INTERRUPT_HPP_ */
