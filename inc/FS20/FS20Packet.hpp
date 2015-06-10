/*
 * FS20Packet.hpp
 *
 *  Created on: Apr 16, 2015
 *      Author: jan
 */

#ifndef FS20PACKET_HPP_
#define FS20PACKET_HPP_

#include "Serial/SerialTx.hpp"
#include "Streams/Streamable.hpp"

namespace FS20 {

using namespace Streams;
using namespace Serial;

struct FS20Packet: public Streamable<FS20Packet> {
    static const uint8_t prefix[];
    static const uint8_t postfix[];

    template <typename prescaled_t>
    static constexpr SerialConfig serialConfig() {
        using namespace TimeUnits;

        return SerialConfig {
            /* highOnIdle */ false,
            prefix, 24,
            highPulse(400_us).template on<prescaled_t>(),
            lowPulse(400_us).template on<prescaled_t>(),
            highPulse(600_us).template on<prescaled_t>(),
            lowPulse(600_us).template on<prescaled_t>(),
            SerialParity::EVEN,
            SerialBitOrder::MSB_FIRST,
            postfix, 2 };
    }

    uint8_t houseCodeHi = 0;
    uint8_t houseCodeLo = 0;
    uint8_t address = 0;
    uint8_t command = 0;
    uint8_t commandExt = 0;
    uint8_t checksum = 0;

    FS20Packet() {}

    FS20Packet(uint8_t _houseCodeHi, uint8_t _houseCodeLo, uint8_t _address, uint8_t _command, uint8_t _commandExt):
        houseCodeHi(_houseCodeHi), houseCodeLo(_houseCodeLo), address(_address), command(_command), commandExt(_commandExt) {
        checksum = getExpectedChecksum();
    }

    bool hasCommandExt() const;
    uint8_t getExpectedChecksum() const;
    bool isChecksumCorrect() const;

    typedef Format<
        Scalar<uint8_t, &FS20Packet::houseCodeHi>,
        Scalar<uint8_t, &FS20Packet::houseCodeLo>,
        Scalar<uint8_t, &FS20Packet::address>,
        Scalar<uint8_t, &FS20Packet::command>,
        Conditional<&FS20Packet::hasCommandExt,
            Scalar<uint8_t, &FS20Packet::commandExt>
        >,
        Scalar<uint8_t, &FS20Packet::checksum>
    > Proto;
};

}

#endif /* FS20PACKET_HPP_ */
