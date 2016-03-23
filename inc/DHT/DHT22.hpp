#ifndef DHT_DHT22_HPP_
#define DHT_DHT22_HPP_

#include "DHT.hpp"

namespace DHT {

namespace Impl {

/**
 * TODO see if this can be power supplied by a pin
 *
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
 * @param pin_t Pin that the DHT22 is connected to. We'll use the internal pull-up resistor of the micro.
 */
template <typename pin_t, typename comparator_t, typename rt_t>
class DHT22: public DHT<pin_t, comparator_t, rt_t> {
    typedef DHT22<pin_t, comparator_t, rt_t> This;
    typedef DHT<pin_t, comparator_t, rt_t> Super;
protected:
    void onComparator() {
        Super::onComparator();
    }
    void onPin() {
        Super::onPin();
    }
public:
    using DHT<pin_t, comparator_t, rt_t>::DHT;

    /**
     * Returns the temperature in tenths of degrees celcius, e.g. 320 for 32 degrees celcius.
     * Can be negative for temperatures below 0.
     */
    int16_t getTemperature() const {
        const uint16_t temp = ((Super::getData(2) & 0x7F) << 8) | Super::getData(3);
        if ((Super::getData(2) & 0x80) != 0) {
            return -temp;
        } else {
            return temp;
        }
    }

    /** Returns the relative humidity in tenths of percent, e.g. 625 for 62.5%. */
    uint16_t getHumidity() const {
        return (Super::getData(0) << 8) | Super::getData(1);
    }

    INTERRUPT_HANDLER1(typename pin_t::INT, onPin);
    INTERRUPT_HANDLER2(typename comparator_t::INT, onComparator);
};

}

template <typename pin_t, typename comparator_t, typename rt_t>
Impl::DHT22<pin_t, comparator_t, rt_t> DHT22(pin_t &pin, comparator_t &comp, rt_t &rt) {
    return { pin, comp, rt };
}

}



#endif /* DHT_DHT11_HPP_ */
