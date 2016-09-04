#ifndef GARDENSENSOR_GARDENSENSOR_HPP_
#define GARDENSENSOR_GARDENSENSOR_HPP_

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/CapacitorRC.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "HopeRF/RFM12.hpp"
#include "Dallas/DS18x20.hpp"
#include "Time/Units.hpp"

/*
  - Big sensor v1:
     Watering   397 -> 410, after 2min 406, after 5 min 404, after 2hr 202, at 24 C
     After 24h  400, still 24 C

     Watering 400 -> 416, at 23 C
*/

#define auto_var(name, expr) decltype(expr) name = expr

namespace GardenSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct GardenSensor {
    typedef GardenSensor<bandgapVoltage,id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<128>(usart0));

    SPIMaster spi;
    ADConverter adc;

    auto_var(timer1, Timer1::withPrescaler<1>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<1024>::inNormalMode());
    auto_var(rt, realTimer(timer1));
    auto_var(nextMeasurement, deadline(rt, 30_s));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinGnd, ArduinoPinA3());
    auto_var(pinCap, ArduinoPinA4().withInterrupt());
    auto_var(pinSupply, ArduinoPinA0());
    auto_var(pinOneWire, ArduinoPinA5());

    auto_var(wire, OneWireParasitePower(pinOneWire, rt));
    auto_var(ds, SingleDS18x20(wire));
    auto_var(soil, (CapacitorRC<4700000>(rt, pinCap)));
    auto_var(rfm, (rfm12<4,128>(spi, pinRFM12_SS, pinRFM12_INT, timer2.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));

	uint8_t seq = 0;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(soil), &This::soil,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(power), &This::power>>>>> Handlers;

    bool measuring = false;

    static constexpr auto RISE_TIME = 500_us;

    void measure() {
        log::debug(F("Measuring DS"));
        ds.measure();
        log::debug(F("Measuring soil"));
        soil.measure();
        measuring = true;
    }

    GardenSensor() {
        pinGnd.configureAsOutputLow();
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
    }

    void loop() {
        //supplyVoltage.stopOnLowBattery(3000);

        if (measuring && !soil.isMeasuring() && !ds.isMeasuring()) {
            pinTX.flush();
            measuring = false;
            log::debug(F("Reading DS"));
            uint16_t temp = ds.getTemperature();
            log::debug(F("Reading Supply 1"));
            for (int i = 0; i < 10; i++) supplyVoltage.get();
            log::debug(F("Reading Supply 2"));
            uint16_t supply = supplyVoltage.get();
            log::debug(F("Reading Soil"));
            auto soilV = soil.getTime();
            log::debug(F("Time : "), dec(soilV));
            pinTX.flush();
            log::debug(F("Suppl: "), dec(supply));
            log::debug(F("Temp : "), dec(temp));
            seq++;
            for (uint8_t i = 3; i > 0; i--) {
                rfm.write_fsk(31, 'O', id, ' ',' ',
                        uint8_t(0),
                        temp,
                        supply,
                        soilV,
                        seq);
            }
            nextMeasurement.schedule();
        } else if (nextMeasurement.isNow()) {
            measure();
        } else {
            auto mode = (rfm.isIdle() && soil.isIdle() && ds.isIdle()) ? SleepMode::POWER_DOWN
                      : SleepMode::IDLE;                    // moisture sensor is running, needs timers
            pinTX.flush();
            power.sleepUntilAny(mode, nextMeasurement, ds);
        }
    }

    int main() {
        while(true) {
        	loop();
        }
    }

};

}


#endif /* GARDENSENSOR_GARDENSENSOR_HPP_ */
