#ifndef STREAMS_PADDING_HPP_
#define STREAMS_PADDING_HPP_

namespace Streams {

/**
 * Indicates padding of [length] bytes. When reading, these bytes will be ignored. When writing,
 * they will be written out as zeroes.
 */
class Padding {
    uint8_t length;
public:
    constexpr Padding (uint8_t l): length(l) {}
    constexpr uint8_t getLength() const {
        return length;
    }
};

}



#endif /* STREAMS_PADDING_HPP_ */
