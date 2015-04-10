#include "FS20Decoder.hpp"

bool FS20Packet::hasCommandExt() const {
    return (command & (1 << 5)) != 0;
}

uint8_t FS20Packet::getExpectedChecksum() const {
    uint8_t sum = uint8_t(6) + houseCodeHi + houseCodeLo + address + command;
    if (hasCommandExt()) {
        sum += commandExt;
    }
    return sum;
}

bool FS20Packet::isChecksumCorrect() const {
    return checksum == getExpectedChecksum();
}
