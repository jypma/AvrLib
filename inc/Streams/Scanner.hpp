/*
 * Scanner.hpp
 *
 *  Created on: Jul 26, 2015
 *      Author: jan
 */

#ifndef SCANNER_HPP_
#define SCANNER_HPP_

#include "Logging.hpp"
#include "ReadResult.hpp"

namespace Streams {

namespace Impl {

using Streams::ReadResult;

template <typename fifo_t>
class Scanner {
    typedef Logging::Log<Loggers::Scanner> log;

    fifo_t * const fifo;
    ReadResult state = ReadResult::Invalid;

public:
    Scanner(fifo_t &_fifo): fifo(&_fifo) {}
    Scanner(const Scanner<fifo_t>& that) = delete;

    /** Returns whether the scanning body should be attempted again */
    bool again() {
        if (state == ReadResult::Invalid && fifo->getReadAvailable() > 0) {
            uint8_t dropped;
            fifo->uncheckedRead(dropped);
            log::debug(F("drp "), dec(dropped));
            return true;
        } else {
            return false;
        }
    }

    template <typename... types>
    bool operator() (types... args) {
        if (state == ReadResult::Valid) {
            // We've already handled a valid case, hence ignore the other branches.
            return false;
        }

        ReadResult result = fifo->read(args...);
        log::debug ('r', dec(uint8_t(result)), 's', dec(uint8_t(state)));
        if (result != ReadResult::Invalid) {
            state = result;
            return state == ReadResult::Valid;
        } else {
            // We've gotten a full or partial match, don't do the other branches.
            return false;
        }
    }
};

}

template<typename fifo_t, typename Body>
void scan(fifo_t &fifo, Body body) {
    typedef Logging::Log<Loggers::Scanner> log;
    if (fifo.isEmpty()) {
        return;
    }

    log::debug(F("S"));

    uint8_t count = 0;
    Impl::Scanner<fifo_t> s(fifo);
    do {
        body(s);
    } while (s.again() && (count++ < 250));

    log::debug(dec(count));
}

}

#endif /* SCANNER_HPP_ */
