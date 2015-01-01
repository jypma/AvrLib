#include "Interrupt.hpp"

void InterruptHandler::attach(volatile void (*_func)(volatile void *), volatile void *_ctx) {
    ScopedNoInterrupts cli;
    func = _func;
    ctx = _ctx;
}

void InterruptHandler::attach(volatile void (*_func)(volatile void *)) {
    attach (_func, nullptr);
}

void InterruptHandler::detach() {
    ScopedNoInterrupts cli;
    func = nullptr;
    ctx = nullptr;
}

void InterruptHandler::invoke() {
    volatile void (*f)(volatile void *);
    volatile void *c;
    {
        ScopedNoInterrupts cli;
        f = func;
        c = ctx;
    }
    if (f != nullptr) {
        f(c);
    }
}
