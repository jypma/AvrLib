#include "InterruptHandler.hpp"
#include "AtomicScope.hpp"

void InterruptHandler::invoke() {
    if (func != nullptr) {
        func(ctx);
    }
}

void InterruptChain::attach(InterruptHandler &handler) {
    AtomicScope _;
    handler.next = head;
    head = &handler;
}

void InterruptChain::detach() {
    AtomicScope _;
    if (head != nullptr) {
        head = head->next;
    }
}

void InterruptChain::invoke() {
    InterruptHandler *handler;
    {
        AtomicScope _;
        handler = head;
    }
    while (handler != nullptr) {
        handler->invoke();
        AtomicScope _;
        handler = handler->next;
    }
}
