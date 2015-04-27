/*
 * FS20Packet.hpp
 *
 *  Created on: Apr 16, 2015
 *      Author: jan
 */

#ifndef FS20PACKET_HPP_
#define FS20PACKET_HPP_

#include "Reader.hpp"
#include "Writer.hpp"
#include "SerialTx.hpp"

struct FS20Packet {
    static constexpr uint8_t prefix[] = { 0b00000000, 0b00010000 };

    template <typename prescaled_t>
    static constexpr SerialConfig serialConfig() {
        using namespace TimeUnits;

        return SerialConfig { false, prefix, 12,
            lowPulse(400_us).template on<prescaled_t>(),
            highPulse(400_us).template on<prescaled_t>(),
            lowPulse(600_us).template on<prescaled_t>(),
            highPulse(600_us).template on<prescaled_t>(),
            true, nullptr, 0 };
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

    static void write(Writer &out, const FS20Packet &packet) {
        out << packet.houseCodeHi << packet.houseCodeLo << packet.address << packet.command;
        if (packet.hasCommandExt()) {
            out << packet.commandExt;
        }
        out << packet.checksum;
    }
    static void read(Reader &in, FS20Packet &packet) {
        in >> packet.houseCodeHi >> packet.houseCodeLo >> packet.address >> packet.command;
        if (packet.hasCommandExt()) {
            in >> packet.commandExt;
        }
        in >> packet.checksum;
    }
};

#endif /* FS20PACKET_HPP_ */
