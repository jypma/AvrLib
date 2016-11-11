#ifndef ROHM_BH1750_HPP_
#define ROHM_BH1750_HPP_

#include <stdint.h>
#include "Enum.hpp"
#include "Option.hpp"
#include "Time/RealTimer.hpp"

#define auto_field(name, expr) decltype(expr) name = expr

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
         * Mode 1 is more appropriate for light areas.
         * BH1750 will power down after measuring.
         */
        oneTimeHighRes = 0x20,

        /**
         * SStart measurement at 0.5 lux resolution. Measurement time is approx 120ms.
         * Mode 2 is more appropriate for dark areas, but will clip when in full light.
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

using namespace Time;

/**
 * Interface to the BH1750 ambient light sensor. Based off https://github.com/claws/BH1750 .
 */
template <typename twi_t, typename rt_t>
class BH1750 {
    constexpr static uint8_t address = 0x23; // for ADDR = low or n/c
                                     //0x5C; // for ADDR = high

    twi_t *const twi;
    rt_t *const rt;
    auto_field(measurementComplete, deadline(*rt, 500_ms));

public:
    void measure(BH1750Mode mode) {
        twi->write(address, mode);
        measurementComplete.schedule();
    }

    bool isMeasuring() {
    	if (measurementComplete.isNow()) {
    		return false;
    	}
    	return measurementComplete.isScheduled();
    }

    bool isIdle() {
    	return !isMeasuring();
    }

    auto timeLeft() const {
    	return measurementComplete.timeLeft();
    }

    BH1750(twi_t &_twi, rt_t &_rt): twi(&_twi), rt(&_rt) {
        measure(BH1750Mode::oneTimeHighRes);
    }

    Option<uint16_t> getLightLevel() {
    	if (isMeasuring()) {
    		return none();
    	}
        uint16_t result;
        if (twi->read(address, &result)) {
        	return result / 1.2;
        } else {
        	return none();
        }
    }
};

}

template <typename twi_t, typename rt_t>
Impl::BH1750<twi_t,rt_t> bh1750(twi_t &twi, rt_t &rt) {
    return { twi, rt };
}

}



#endif /* ROHM_BH1750_HPP_ */
