#ifndef DHT_DHT11_HPP_
#define DHT_DHT11_HPP_

#include "DHT.hpp"

namespace DHT {

namespace Impl {

/**
 * TODO see if this can be power supplied by a pin
 *
 * DHT11 driver. Gets a measurement on startup after which it goes to idle. Call measure() to initiate another measurement.
 *
 *      /------------\
 *      |            |
 *      |   DHT11    |
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
 * @param pin_t Pin that the DHT11 is connected to. We'll use the internal pull-up resistor of the micro.
 */
template <typename pin_t, typename comparator_t, typename rt_t>
class DHT11: public DHT<pin_t, comparator_t, rt_t> {
    typedef DHT11<pin_t, comparator_t, rt_t> This;
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
     * However, the DHT11's precision is whole degrees only.
     */
    int16_t getTemperature() const {
        return Super::getData(2) * 10;
    }

    /**
     * Returns the relative humidity in tenths of percent, e.g. 625 for 62.5%.
     * However, the DHT11's precision is whole percentages only.
     */
    uint16_t getHumidity() const {
        return Super::getData(0) * 10;
    }

    INTERRUPT_HANDLER1(typename pin_t::INT, onPin);
    INTERRUPT_HANDLER2(typename comparator_t::INT, onComparator);
};

}

template <typename pin_t, typename comparator_t, typename rt_t>
Impl::DHT11<pin_t, comparator_t, rt_t> DHT11(pin_t &pin, comparator_t &comp, rt_t &rt) {
    return { pin, comp, rt };
}

}



#endif /* DHT_DHT11_HPP_ */
