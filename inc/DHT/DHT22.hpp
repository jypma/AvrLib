#ifndef DHT_DHT22_HPP_
#define DHT_DHT22_HPP_

#include "DHT.hpp"

namespace DHT {

namespace Impl {

/**
 * DHT22 driver. Gets a measurement on startup after which it goes to idle. Call measure() to initiate another measurement.
 *
 *      /------------\
 *      |            |
 *      |   DHT22    |
 *      |  (front)   |
 *      |            |
 *      |            |
 *      \-+--+--+--+-/
 *        |  |  |  |
 *        1  2  3  4
 *        |  |  x  |
 *     VCC/  |     \GND
 *           \DATA
 *
 * @param datapin_t DATA pin. We'll use the internal pull-up resistor of the micro.
 * @param powerpin_t VCC pin. The sensor will be powered by the microcontroller.
 */
template <typename datapin_t, typename powerpin_t, typename comparator_t, typename rt_t>
class DHT22: public DHT<datapin_t, powerpin_t, comparator_t, rt_t> {
    typedef DHT22<datapin_t, powerpin_t, comparator_t, rt_t> This;
    typedef DHT<datapin_t, powerpin_t, comparator_t, rt_t> Super;
    using Super::getLastFailure;
public:
    using Super::DHT;

    /**
     * Returns the temperature in tenths of degrees celcius, e.g. 320 for 32 degrees C or -15 for -1.5 degrees C.
     * Can be negative for temperatures below 0.
     */
    Option<int16_t> getTemperature() const {
    	if (getLastFailure() == 0) {
            const uint16_t temp = ((Super::getData(2) & 0x7F) << 8) | Super::getData(3);
            if ((Super::getData(2) & 0x80) != 0) {
                return -temp;
            } else {
                return temp;
            }
    	} else {
    		return none();
    	}
    }

    /** Returns the relative humidity in tenths of percent, e.g. 625 for 62.5%. */
    Option<uint16_t> getHumidity() const {
    	if (getLastFailure() == 0) {
    		return (Super::getData(0) << 8) | Super::getData(1);
    	} else {
    		return none();
    	}
    }
};

}

template <typename datapin_t, typename powerpin_t, typename comparator_t, typename rt_t>
Impl::DHT22<datapin_t, powerpin_t, comparator_t, rt_t> DHT22(datapin_t &pin, powerpin_t &power, comparator_t &comp, rt_t &rt) {
    return { pin, power, comp, rt };
}

}



#endif /* DHT_DHT11_HPP_ */
