#ifndef STREAMS_NESTED_HPP_
#define STREAMS_NESTED_HPP_

namespace Streams {
namespace Impl {

template <typename lambda_t>
class Nested {
    lambda_t lambda;
public:
    constexpr Nested(lambda_t l): lambda(l) {}

    template <typename... types>
    auto operator () (types... args) {
        return lambda(args...);
    }
};

template <typename fifo_t>
struct doread;

template <typename sem, typename fifo_t>
struct dowrite;

}

/**
 * Registers a lambda invocation as nested read/write The lambda takes one argument
 * which, when invoked with (...), can be used just like read() or write(). The lambda
 * must return a ReadResult for read, or a bool for write.
 */
template <typename lambda_t>
Impl::Nested<lambda_t> Nested(lambda_t lambda) {
    return lambda;
}

}



#endif /* STREAMS_NESTED_HPP_ */
