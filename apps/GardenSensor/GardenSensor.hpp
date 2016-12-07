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

#define auto_var(name, expr) decltype(expr) name = expr

namespace GardenSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;

struct Measurement {
	Option<uint32_t> soil;
	Option<int16_t> temp;
	Option<uint16_t> supply;
	uint8_t seq;
	uint16_t sender;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, Option<uint16_t>, &Measurement::supply>,
		P::Varint<10, Option<int16_t>, &Measurement::temp>,
		P::Varint<11, Option<uint32_t>, &Measurement::soil>
	> DefaultProtocol;
};

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct GardenSensor {
    typedef GardenSensor<bandgapVoltage,id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<128>(usart0));

    SPIMaster spi;
    ADConverter<uint16_t> adc;

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
        log::debug(F("Measuring"));
        ds.measure();
        soil.measure();
        measuring = true;
    }

    GardenSensor() {
        pinGnd.configureAsOutputLow();
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
        nextMeasurement.cancel();
    }

    auto getTaskState() {
    	return TaskState(nextMeasurement, SleepMode::POWER_DOWN);
    }

	Measurement m = {};
    void loop() {
        supplyVoltage.stopOnLowBattery(3000);
        auto soilState = soil.getTaskState();
        auto dsState = ds.getTaskState();
        auto measureState = getTaskState();
        auto rfmState = rfm.getTaskState();

        if (measuring && soilState.isIdle() && dsState.isIdle()) {
            pinTX.flush();
            measuring = false;
            log::debug(F("Reading DS"));
            m.temp = ds.getTemperature();
            log::debug(F("Reading Supply 1"));
            for (int i = 0; i < 10; i++) supplyVoltage.get();
            log::debug(F("Reading Supply 2"));
            m.supply = supplyVoltage.get();
            log::debug(F("Reading Soil"));
            m.soil = soil.getValue();
            log::debug(F("Soil : "), dec(m.soil));
            pinTX.flush();
            log::debug(F("Suppl: "), dec(m.supply));
            pinTX.flush();
            log::debug(F("Temp : "), dec(m.temp));
            if (m.supply > 4500U || m.supply < 2000U) {
            	// Don't log if we're on AC, or if the value is non-sensical.
            	m.supply = none();
            }
            seq++;
            m.seq = seq;
            m.sender = 'O' << 8 | read(id);
            for (uint8_t i = 3; i > 0; i--) {
                rfm.write_fsk(42, &m);
            }
            nextMeasurement.schedule();
        } else if (nextMeasurement.isNow()) {
            measure();
        } else {
        	power.sleepUntilTasks(dsState, soilState, rfmState, measureState); // , nextMeasurement...
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
