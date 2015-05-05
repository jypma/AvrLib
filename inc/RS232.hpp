/*
 * RS232.hpp
 *
 *  Created on: May 2, 2015
 *      Author: jan
 */

#ifndef RS232_HPP_
#define RS232_HPP_

#include "SerialTx.hpp"

struct RS232 {
private:
    static const uint8_t prefix[];
    static const uint8_t postfix[];
public:
    template <typename prescaled_t, uint32_t bits_per_second = 57600>
    static SerialConfig serialConfig_8n1() {
        using namespace TimeUnits;

        // TODO put static_assert here that errors out on bit rate errors > 3% ???
        // Or do varying pulse lenghts
        constexpr auto bit_length = prescaled_t::template microseconds2counts<1000000 / bits_per_second>();

        return SerialConfig {
            /* highOnIdle */ true,
            prefix, 1,
            Pulse(true, bit_length),
            Pulse::empty(),
            Pulse(false, bit_length),
            Pulse::empty(),
            SerialParity::NONE,
            SerialBitOrder::LSB_FIRST,
            postfix, 1 };
    }
};

#endif /* RS232_HPP_ */
