/*
 * Interrupt.hpp
 *
 *  Created on: Dec 26, 2014
 *      Author: jan
 */

#ifndef INTERRUPT_HPP_
#define INTERRUPT_HPP_

class InterruptChain;

class InterruptHandler {
    friend class InterruptChain;

    void (* const func)(volatile void *);
    volatile void * const ctx;

    InterruptHandler *next = nullptr;

public:
    // This is safe, according to https://gcc.gnu.org/onlinedocs/gcc-3.4.5/gcc/Bound-member-functions.html
    // But, you should compile using -Wno-pmf-conversions.
    template <typename T>
    InterruptHandler(T* _ctx, void ((T::*_func)())): func((void (*)(volatile void *)) _func), ctx(_ctx) {}

    InterruptHandler(void (*_func)(volatile void *), volatile void *_ctx): func(_func), ctx(_ctx) {}

    InterruptHandler(void (*_func)(volatile void *)): func(_func), ctx(nullptr) {}

    /** Invokes this handler. */
    void invoke();
};

class InterruptChain {
    InterruptHandler *head = nullptr;
    InterruptHandler *getHead();

public:
    /**
     * Attaches the given interrupt handler. The handler instance must be permanently valid
     * during the time the handler is attached. This is best done by making it a field in
     * your class.
     *
     * Note that an interrupt handler can only be attached to one InterruptChain.
     */
    void attach(InterruptHandler &handler);

    /**
     * Detaches the last-registered interrupt handler.
     */
    void detach();

    /**
     * Invokes all interrupt handlers in the chain.
     */
    void invoke();
};

#endif /* INTERRUPT_HPP_ */
