#include "Streams/ReadingHexadecimal.hpp"

namespace Streams {
namespace Impl {

int8_t fromHex(uint8_t ch) {
    return ((ch >= '0') && (ch <= '9')) ? (ch - '0') :
           ((ch >= 'a') && (ch <= 'f')) ? (ch + 10 - 'a') :
           ((ch >= 'A') && (ch <= 'F')) ? (ch + 10 - 'A') :
           -1;
}

}
}
