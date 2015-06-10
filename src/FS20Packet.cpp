#include "FS20/FS20Packet.hpp"

using namespace FS20;

const uint8_t FS20Packet::prefix[] = { 0b00000000, 0b00000000, 0b10000000 };

const uint8_t FS20Packet::postfix[] = { 0b00, };
