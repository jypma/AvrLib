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
protected:
    /** Invokes any registered handler */
    void invoke();
public:
    void attach(void (*_func)(volatile void *), volatile void *_ctx);
    void attach(void (*_func)(volatile void *));
    void detach();
};

#endif /* INTERRUPT_HPP_ */
