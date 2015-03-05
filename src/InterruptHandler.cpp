#include "InterruptHandler.hpp"
#include "AtomicScope.hpp"

InterruptHandler InterruptHandler::attach(void (*_func)(volatile void *), volatile void *_ctx) {
    AtomicScope _;
    InterruptHandler result = *this;
    func = _func;
    ctx = _ctx;
    return result;
}

InterruptHandler InterruptHandler::attach(void (*_func)(volatile void *)) {
    return attach (_func, nullptr);
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
