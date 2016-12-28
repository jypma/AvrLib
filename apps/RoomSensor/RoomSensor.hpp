#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "HopeRF/RFM12.hpp"
#include "Dallas/DS18x20.hpp"
#include "Time/Units.hpp"
#include "DHT/DHT22.hpp"
#include "Option.hpp"
#include "ROHM/BH1750.hpp"
#include "PIR/HCSR501.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

volatile uint8_t hcr_ints = 0;

namespace RoomSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;
using namespace DHT;
using namespace ROHM;
using namespace PIR;

struct Measurement {
	Option<int16_t> temp;
	Option<uint16_t> humidity;
	Option<uint16_t> supply;
	uint8_t seq;
	uint16_t sender;
	Option<uint16_t> lux;
	Option<uint8_t> motion;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, Option<uint16_t>, &Measurement::supply>,
		P::Varint<10, Option<int16_t>, &Measurement::temp>,
		P::Varint<11, Option<uint16_t>, &Measurement::humidity>,
		P::Varint<12, Option<uint16_t>, &Measurement::lux>,
	    P::Varint<13, Option<uint8_t>, &Measurement::motion>
	> DefaultProtocol;
};

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct RoomSensor {
    typedef RoomSensor<bandgapVoltage,id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<100>(usart0));
    auto_var(pinRX, PinPD0());

    SPIMaster spi;
    ADConverter<uint16_t> adc;

    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(timer1, Timer1::withPrescaler<1>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(nextMeasurement, deadline(rt, 30_s));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinSupply, JeeNodePort2A());
    auto_var(pinDHT, JeeNodePort2D().withInterrupt());
    auto_var(pinDHTPower, JeeNodePort3A());
    auto_var(pinPIRPower, JeeNodePort4D());
    auto_var(pinPIRData, PinPD3());

    auto_var(rfm, (rfm12<4,100>(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(dht, DHT22(pinDHT, pinDHTPower, timer0.comparatorB(), rt));
    auto_var(pir, HCSR501(pinPIRData, pinPIRPower, rt));
    TWI<> twi = {};
    auto_var(bh, bh1750(twi, rt));

	uint8_t seq = 0;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(dht), &This::dht,
            Delegate<This, decltype(rfm), &This::rfm,
			Delegate<This, decltype(twi), &This::twi,
			Delegate<This, decltype(pir), &This::pir,
            Delegate<This, decltype(power), &This::power
			>>>>>>> Handlers;

    bool measuring = false;

    void measure() {
        log::debug(F("Measuring"));
        log::flush();
        dht.measure();
        bh.measure(BH1750Mode::oneTimeHighRes);
        measuring = true;
    }

    RoomSensor() {
    	pinRX.configureAsInputWithPullup();
        log::debug(F("Starting"));
        rfm.onIdleSleep();
    }

    auto getTaskState() {
    	return TaskState(nextMeasurement, SleepMode::POWER_DOWN);
    }

    void loop() {
        supplyVoltage.stopOnLowBattery(3000, [&] {
        	log::debug(F("**LOW**"));
        	log::flush();
        });
        dht.loop();
        pir.loop();
        bh.loop();

        if (pir.isMotionDetected()) {
        	Measurement m = {};
        	log::debug(F("ints: "), dec(pir.getInts()));
            log::debug(F("Motion!"));
        	m.motion = 1;
            seq++;
            m.seq = seq;
            m.sender = 'Q' << 8 | read(id);
            rfm.write_fsk(42, &m);
        }

        if (rfm.isIdle()) {
        	pir.enable();
        }

        auto dhtState = dht.getTaskState();
        auto bhState = bh.getTaskState();
        auto rfmState = rfm.getTaskState();
        auto pirState = pir.getTaskState();
        auto measureState = getTaskState();

        if (measuring && dhtState.isIdle() && bhState.isIdle()) {
        	Measurement m = {};
        	log::debug(F("ints: "), dec(pir.getInts()));
            pinTX.flush();
            measuring = false;

            for (int i = 0; i < 10; i++) supplyVoltage.get();
            m.supply = supplyVoltage.get();
            log::debug(F("Suppl: "), dec(m.supply));

            m.humidity = dht.getHumidity();
            log::debug(F("Hum  : "), dec(m.humidity));

            m.temp = dht.getTemperature();
            log::debug(F("Temp : "), dec(m.temp));
            log::flush();

            m.lux = bh.getLightLevel();
            log::debug(F("Lux  : "), dec(m.lux));
        	log::flush();

        	seq++;
            m.seq = seq;
            m.sender = 'Q' << 8 | read(id);
            pir.disable();
            rfm.write_fsk(42, &m);
            nextMeasurement.schedule();
        } else if (nextMeasurement.isNow()) {
            measure();
        } else {
        	power.sleepUntilTasks(dhtState, bhState, rfmState, measureState, pirState);
        }
    }

    int main() {
        while(true) {
        	loop();
        }
    }

};

}
