#include "InterruptHandler.hpp"
#include "AtomicScope.hpp"

void InterruptHandler::attach(void (*_func)(volatile void *), volatile void *_ctx) {
    AtomicScope _;
    func = _func;
    ctx = _ctx;
}

void InterruptHandler::attach(void (*_func)(volatile void *)) {
    attach (_func, nullptr);
}

void InterruptHandler::detach() {
    AtomicScope _;
    func = nullptr;
    ctx = nullptr;
}

void InterruptHandler::invoke() {
    void (*f)(volatile void *);
    volatile void *c;
    {
        AtomicScope _;
        f = func;
        c = ctx;
    }
    if (f != nullptr) {
        f(c);
    }
}
