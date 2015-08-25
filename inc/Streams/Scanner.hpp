/*
 * Scanner.hpp
 *
 *  Created on: Jul 26, 2015
 *      Author: jan
 */

#ifndef SCANNER_HPP_
#define SCANNER_HPP_

#include "Reader.hpp"
#include "Logging.hpp"

namespace Streams {

struct NoTarget {};

template <typename fifo_t, typename target_t>
class ScannerSemantics {
    target_t *target;
public:
    ScannerSemantics(target_t &t): target(&t) {}

    template <typename format>
    inline ReaderState scan(fifo_t &fifo) const {
        return fifo.template readAs<format>(*target);
    }
};

template<typename fifo_t>
class ScannerSemantics<fifo_t, NoTarget> {
public:
    template <typename format>
    inline ReaderState scan(fifo_t &fifo) const {
        return fifo.template expect<format>();
    }
};

template <typename fifo_t, typename sem_t>
class Scanner {
    typedef Logging::Log<Loggers::Streams> log;

    fifo_t *fifo;
    uint8_t available;
    ReaderState state = ReaderState::Invalid;
    const sem_t semantics;
public:
    Scanner(fifo_t &f, const sem_t s): fifo(&f), semantics(s) {
        available = fifo->getReadAvailable();
    }

    template <typename format, typename F>
    void on(F f) {
        if (state == ReaderState::Valid) {
            // We've already handled a valid case, hence ignore the other branches.
            return;
        }

        ReaderState thisResult = semantics.template scan<format>(*fifo);

        if (thisResult == ReaderState::Valid) {
            state = ReaderState::Valid;
            f();
        } else if (thisResult == ReaderState::Incomplete) {
            state = ReaderState::Incomplete;
        } else if (thisResult == ReaderState::Partial) {
            state = ReaderState::Partial;
        }
    }

    bool notDone() {
        if (available > 0 && state == ReaderState::Invalid) {
            uint8_t dropped;
            fifo->uncheckedRead(dropped);
            log::debug("  state invalid, dropped %c\n", dropped);
            available--;
            return true;
        } else {
            return false;
        }
    }
};

template<typename fifo_t>
Scanner<fifo_t,ScannerSemantics<fifo_t,NoTarget>> scanner(fifo_t &fifo) {
    return Scanner<fifo_t,ScannerSemantics<fifo_t,NoTarget>>(fifo, ScannerSemantics<fifo_t,NoTarget>());
}

template<typename fifo_t, typename target_t>
Scanner<fifo_t,ScannerSemantics<fifo_t,target_t>> scanner(fifo_t &fifo, target_t &target) {
    return Scanner<fifo_t,ScannerSemantics<fifo_t,target_t>>(fifo, ScannerSemantics<fifo_t,target_t>(target));
}

template<typename fifo_t, typename Body>
void scan(fifo_t &fifo, Body body) {
    auto s = scanner(fifo);
    do {
        body(&s);
    } while (s.notDone());
}

template<typename fifo_t, typename target_t, typename Body>
void scan(fifo_t &fifo, target_t &target, Body body) {
    auto s = scanner(fifo, target);
    do {
        body(&s);
    } while (s.notDone());
}

template<typename format, typename scanner_t, typename lambda_t>
inline void on(scanner_t *scanner, lambda_t f) {
    scanner->template on<format>(f);
}

}

#endif /* SCANNER_HPP_ */
