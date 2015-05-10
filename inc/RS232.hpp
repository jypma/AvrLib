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
    struct _8n1 {
        // TODO put static_assert here that errors out on bit rate errors > 3% ???
        // Or do varying pulse lenghts
        static constexpr typename prescaled_t::value_t bit_length = prescaled_t::template microseconds2counts<1000000l / bits_per_second>();
        static_assert(bit_length > prescaled_t::template microseconds2counts<30>(), "bits_per_second must be low enough so that a single bit is more than 25us, since the software serial implementation is so slow.");

        static constexpr SerialConfig serialconfig = SerialConfig {
            /* highOnIdle */ true,
            prefix, 1,
            Pulse(false, bit_length),
            Pulse::empty(),
            Pulse(true, bit_length),
            Pulse::empty(),
            SerialParity::NONE,
            SerialBitOrder::LSB_FIRST,
            postfix, 1 };
    };
};

#endif /* RS232_HPP_ */
