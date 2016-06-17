#ifndef ROHM_BH1750_HPP_
#define ROHM_BH1750_HPP_

#include <stdint.h>
#include "TWI.hpp"
#include "Enum.hpp"

namespace ROHM {

namespace E {

struct BH1750Mode {
    enum type: uint8_t {
        /**
         * Start measurement at 1 lux resolution. Measurement time is approx 120ms.
         */
        continuousHighRes = 0x10,

        /**
         * Start measurement at 0.5 lux resolution. Measurement time is approx 120ms.
         */
        continuousHighRes2 = 0x11,

        /**
         * Start measurement at 4 lux resolution. Measurement time is approx 16ms.
         */
        continuousLowRes = 0x13,

        /**
         * Start measurement at 1 lux resolution. Measurement time is approx 120ms.
         * BH1750 will power down after measuring.
         */
        oneTimeHighRes = 0x20,

        /**
         * SStart measurement at 0.5 lux resolution. Measurement time is approx 120ms.
         * BH1750 will power down after measuring.
         */
        oneTimeHighRes2 = 0x21,

        /**
         * Start measurement at 4 lux resolution. Measurement time is approx 16ms.
         * BH1750 will power down after measuring.
         */
        oneTimeLowRes = 0x23
    };
};

}

class BH1750Mode: public Enum<E::BH1750Mode> {
public:
    typedef E::BH1750Mode type;
    using Enum<E::BH1750Mode>::Enum;
};

namespace Impl {

/**
 * Interface to the BH1750 ambient light sensor. Based off https://github.com/claws/BH1750 .
 */
template <typename twi_t, typename rt_t>
class BH1750 {
    constexpr static uint8_t address = 0x23;

    twi_t *const twi;
    rt_t *const rt;

public:
    void configure(BH1750Mode mode) {
        twi->write(address, mode);
    }

    BH1750(twi_t &_twi, rt_t &_rt): twi(&_twi), rt(&_rt) {
        configure(BH1750Mode::continuousHighRes);
    }

    uint16_t readLevel() {
        uint16_t result;
        twi->read(address, &result);
        return result / 1.2;
    }
};

}

template <typename twi_t, typename rt_t>
Impl::BH1750<twi_t,rt_t> bh1750(twi_t &twi, rt_t &rt) {
    return { twi, rt };
}

}



#endif /* ROHM_BH1750_HPP_ */
