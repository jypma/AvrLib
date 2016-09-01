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

// stuck with battery drop: 07-24 1:28:00 till 03:06:40 (2hr - 22min + 40sec = 1hr 38min 40sec = 5920 sec
//     problem is realtimer with prescaler 1?  -> use rt with timer2 instead.
// stuck with battery drop: 07-25 6:30:40 till 7:43:00 (1hr 12min 20sec) ... not the same.

// big rain 07-29 14:23

// stuck with 2x rt:
// .*Main: s=1 d=779541238 t=4294967295
// .*Main: s=1 d=779430897 t=4294967295

// stuck: with 1x rt:
// *Main: s=3 d=2745380360 t=4294967295
// *Main: s=3 d=2745268155 t=4294967295


/*
Tue 23 Aug 2016 07:08:36 CEST:  Main: Reading Supply 2
Tue 23 Aug 2016 07:08:36 CEST:  Main: Reading Soil
Tue 23 Aug 2016 07:08:36 CEST:  Main: Time : 3581
Tue 23 Aug 2016 07:08:36 CEST:  Main: Suppl: 5112
Tue 23 Aug 2016 07:08:36 CEST:  Main: Temp : 245
Tue 23 Aug 2016 07:08:36 CEST:  *Main: s=0 d=4294967295 t=4294967295
Tue 23 Aug 2016 07:08:36 CEST:  *Main: s=0 d=4294967295 t=4294967295
Tue 23 Aug 2016 07:08:36 CEST:  *Main: s=0 d=4294967295 t=4294967295
Tue 23 Aug 2016 07:08:36 CEST:  *Main: s=0 d=4294967295 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  s*Main: s=0 d=4294967295 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  s*Main: Measuring DS
Tue 23 Aug 2016 07:09:06 CEST:  Main: Measuring soil
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996653
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4319 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996653
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4322 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996745
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4335 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996732
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4311 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996985
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4313 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996749
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4332 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996753
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4311 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996997
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4329 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996744
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4328 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996750
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4326 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996737
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4311 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996744
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4326 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996997
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4326 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996759
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4329 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996735
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4311 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996738
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4317 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996735
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4308 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996997
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4308 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996744
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4320 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996729
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4317 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996738
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4308 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996741
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4323 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996994
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=1 d=4323 t=4294967295
Tue 23 Aug 2016 07:09:06 CEST:  *Main: s=2 d=4294967295 t=15996741
[...]
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4296 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996741
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4302 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996991
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4302 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996741
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4302 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996738
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4302 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996723
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4305 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996726
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4317 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996744
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4299 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996644
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4305 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996738
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4311 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996757
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4339 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996748
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4336 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996748
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4336 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996760
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4339 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996769
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4357 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996748
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4336 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996775
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4342 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996760
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4342 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996754
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=1 d=4336 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=2 d=4294967295 t=15996769
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294869620 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294757945 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294646356 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294534755 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294423131 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294311510 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294199892 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4294088291 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293976684 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293865051 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293753433 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293641803 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293530199 t=4294967295
Tue 23 Aug 2016 07:09:07 CEST:  *Main: s=3 d=4293418835 t=4294967295
T
 */

#define let(name, expr) decltype(expr) name = expr

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
    let(pinTX, PinPD1<128>(usart0));

    SPIMaster spi;
    ADConverter adc;

    let(timer0, Timer0::withPrescaler<1>::inNormalMode());
    let(timer2, Timer2::withPrescaler<1024>::inNormalMode());
    //let(rt_hires, realTimer(timer0));
    let(rt, realTimer(timer0));
    let(nextMeasurement, deadline(rt, 30_s));

    let(pinRFM12_INT, PinPD2());
    let(pinRFM12_SS, PinPB2());
    let(pinGnd, ArduinoPinA3());
    let(pinCap, ArduinoPinA4().withInterrupt());
    let(pinSupply, ArduinoPinA0());
    let(pinOneWire, ArduinoPinA5());

    let(wire, OneWireParasitePower(pinOneWire, rt));
    let(ds, SingleDS18x20(wire));
    let(soil, (CapacitorRC<4700000>(rt, pinCap)));
    let(rfm, (rfm12<4,128>(spi, pinRFM12_SS, pinRFM12_INT, timer2.comparatorA(), RFM12Band::_868Mhz)));
    let(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    let(power, Power(rt));

    volatile uint8_t tstCh = 0;

    void onTst() { tstCh++; }

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
            //Delegate<This, decltype(rt_hires), &This::rt_hires,
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

    int main() {
        pinGnd.configureAsOutputLow();
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
        uint8_t seq = 0;
        while(true) {
            pinTX.write('*');
            pinTX.flush();
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

                auto nLeft = toMillisOn(rt, nextMeasurement.timeLeft());
                auto dLeft = toMillisOn(rt, ds.timeLeft());
                auto min = (nLeft < dLeft) ? nLeft : dLeft;
                if (mode == SleepMode::IDLE && min > 8000_ms) {
                    min = 8000_ms;
                }
                log::debug(F("s="), dec(uint8_t(soil.getState())), F(" d="), dec(soil.getChargeDone().timeLeft().getValue()),
                		F(" t="), dec(soil.getTimeout().timeLeft().getValue()),
						F(" C="), dec(rt.counts().getValue()),
						F(" T="), dec(rt.ticks().getValue())
						);
                pinTX.flush();
                bool i = power.sleepFor(min, mode);
                if (mode == SleepMode::IDLE) {
                    if (!i) pinTX.write('.');
                } else {
                    pinTX.write(i ? 'S' : 's');
                }
                pinTX.flush();
            }
        }
    }

};

}


#endif /* GARDENSENSOR_GARDENSENSOR_HPP_ */
