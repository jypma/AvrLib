#ifndef DALLAS_DS18X20_HPP_
#define DALLAS_DS18X20_HPP_

#include "OneWire.hpp"
#include "Option.hpp"
#include <stdint.h>
#include <Tasks/TaskState.hpp>

namespace Dallas {

namespace Impl {

using namespace Time;
using namespace Streams;

/**
 * Dallas DS18B20 temperature sensor
 *
 * Top view (curve front) : (note that the datasheet shows a _bottom_ view, which is the reverse)
 *
 *        +-----------------+
 *        |\               /|
 *        | --___     ___-- |
 *        |      ^^^^^      |
 *        |                 |
 *        |    DS18B20      |
 *        |                 |
 *         \               /
 *          --___     ___--
 *           |   ^^^^^   |
 *           |     |     |
 *       VCC |     |     | GND
 *           |     DATA  |
 *           +-4.7K+     |
 *           |     |     |
 *
 * You must connect a 4.7K pull-up resistor between DATA and VCC.
 * The internal pull-up of the ATMega328 is not sufficient to use the device.
 *
 *------------------------------------------------------------------------------------------------
 *
 * You can also power the DS18B20 using parasite power. This way, you only have to run 2 wires.
 * For this, connect pin 1 to 3 on the device side. On the micro side, you still need a 4.7K pull-up.
 *
 *        +-----------------+
 *        |\               /|
 *        | --___     ___-- |
 *        |      ^^^^^      |
 *        |                 |
 *        |    DS18B20      |
 *        |                 |
 *         \               /
 *          --___     ___--
 *           |   ^^^^^   |
 *           |     |     |
 *           |     |     | GND
 *           |     DATA  |
 *           |     |     |
 *           \-----n-----+
 *                 |     |
 *  VCC            |     |
 *   |             |     |
 *    \--[ 4.7K]---+     |
 *                 |     |
 *
 */
template<typename onewire_t>
class DS18x20 {
    typedef typename onewire_t::rt_t rt_t;
    typedef Logging::Log<Loggers::Dallas> log;

    onewire_t * const wire;
    const OneWireAddress addr;
    Deadline<rt_t,decltype(1200_ms)> measureDone = { *wire->rt };
    Option<int16_t> temp = none();

    void readTemperature() {
        log::debug(F("Retrieving temp"));
        if (!wire->reset()) {
        	temp = none();
        	return;
        }
        wire->select(addr);
        wire->write(0xBE); // READ SCRATCHPAD
        uint8_t data[9];
        for (uint8_t i = 0; i < 9; i++) {
            data[i] = wire->read();
        }

        int16_t raw = (data[1] << 8) | data[0];
        if (addr.addr[0] == 0x10) { // DS18S20, or old DS1820
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10) {
                // "count remain" gives full 12 bit resolution
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
        } else {
            uint8_t cfg = (data[4] & 0x60);
            // at lower res, the low bits are undefined, so let's zero them
            if (cfg == 0x00)
                raw = raw & ~7; // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20)
                raw = raw & ~3; // 10 bit res, 187.5 ms
            else if (cfg == 0x40)
                raw = raw & ~1; // 11 bit res, 375 ms
            //// default is 12 bit resolution, 750 ms conversion time
        }

        temp = (int32_t(raw) * 100) / 160;
    }

    void update() {
        if (measureDone.isNow()) {
            readTemperature();
        }
    }

public:
    DS18x20(onewire_t &w, const OneWireAddress &a): wire(&w), addr(a) {
        measureDone.cancel();
    }

    /**
     * Starts a measurement. The result can be read after 1 second.
     */
    void measure() {
        if (measureDone.isScheduled()) {
            return;
        }
        if (!wire->reset()) {
        	if (!wire->reset()) {
        		measureDone.cancel();
        		temp = none();
        		return;
        	}
        }
        wire->select(addr);
        wire->write(0x44); // CONVERT T
        measureDone.schedule();
    }

    /** Returns how much time there is left for until any ongoing measurement completes. */
    auto timeLeft() const {
        return measureDone.timeLeft();
    }

    /**
     * Returns whether a measurement is currently in progress.
     */
    bool isMeasuring() {
        update();
        return measureDone.isScheduled();
    }

    bool isIdle() {
        return !isMeasuring();
    }

    /**
     * Returns the temperature in tenths of degrees celcius, e.g. 320 for 32 degrees C or -15 for -1.5 degrees C.
     * Can be negative for temperatures below 0.
     */
    Option<int16_t> getTemperature() {
        update();
        return temp;
    }

    auto getTaskState() {
    	update();
    	return TaskState(measureDone, HAL::Atmel::SleepMode::POWER_DOWN);
    }
};

} // namespace Impl

template<typename onewire_t>
Impl::DS18x20<onewire_t> SingleDS18x20(onewire_t &wire) {
    OneWireAddress addr;
    wire.search(addr);
    return { wire, addr };
}

} // namespace Dallas



#endif /* DALLAS_DS18X20_HPP_ */
