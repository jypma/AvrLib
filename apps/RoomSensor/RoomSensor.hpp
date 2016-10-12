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

#define auto_var(name, expr) decltype(expr) name = expr

namespace RoomSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;
using namespace DHT;
using namespace ROHM;

struct Measurement {
	Option<int16_t> temp;
	Option<uint16_t> humidity;
	uint16_t supply;
	uint8_t seq;
	uint16_t sender;
	Option<uint16_t> lux;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, uint16_t, &Measurement::supply>,
		P::Varint<10, Option<int16_t>, &Measurement::temp>,
		P::Varint<11, Option<uint16_t>, &Measurement::humidity>,
		P::Varint<12, Option<uint16_t>, &Measurement::lux>
	> DefaultProtocol;
};

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct RoomSensor {
    typedef RoomSensor<bandgapVoltage,id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<128>(usart0));

    SPIMaster spi;
    ADConverter adc;

    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(timer1, Timer1::withPrescaler<1>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(nextMeasurement, deadline(rt, 5_s));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinSupply, JeeNodePort2A());
    auto_var(pinOneWire, JeeNodePort3D());
    auto_var(pinDHT, JeeNodePort2D().withInterrupt());
    auto_var(pinDHTPower, JeeNodePort3A());

    auto_var(wire, OneWireParasitePower(pinOneWire, rt));
    auto_var(ds, SingleDS18x20(wire));
    auto_var(rfm, (rfm12<4,128>(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(dht, DHT22(pinDHT, pinDHTPower, timer0.comparatorB(), rt));
    TWI<> twi = {};
    auto_var(bh, bh1750(twi, rt));

	uint8_t seq = 0;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(dht), &This::dht,
            Delegate<This, decltype(rfm), &This::rfm,
			Delegate<This, decltype(twi), &This::twi,
            Delegate<This, decltype(power), &This::power>>>>>> Handlers;

    bool measuring = false;

    void measure() {
        log::debug(F("Measuring"));
        log::flush();
        ds.measure();
        dht.measure();
        bh.measure(BH1750Mode::oneTimeHighRes);
        measuring = true;
    }

    RoomSensor() {
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
    }

    Measurement m;
    void loop() {
        supplyVoltage.stopOnLowBattery(3000, [&] {
        	log::debug(F("**LOW**"));
        	log::flush();
        });
        dht.loop();
        pinTX.flush();
        Logging::printTimings();
        //pinTX.write(dec(pls));
        pinTX.flush();

        if (measuring && !dht.isMeasuring() && !ds.isMeasuring() && !bh.isMeasuring()) {
            pinTX.flush();
            measuring = false;
            log::debug(F("Reading DS"));
            m.temp = ds.getTemperature();
            log::debug(F("Reading Supply 1"));
            for (int i = 0; i < 10; i++) supplyVoltage.get();
            log::debug(F("Reading Supply 2"));
            m.supply = supplyVoltage.get();
            log::debug(F("Reading DHT"));
			m.humidity = dht.getHumidity();
            log::debug(F("Hum  : "), dec(m.humidity));
            pinTX.flush();
            m.lux = bh.getLightLevel();
            log::debug(F("Suppl: "), dec(m.supply));
            log::debug(F("Temp : "), dec(m.temp));
            log::debug(F("TempH: "), dec(dht.getTemperature()));
            log::debug(F("Lux  : "), dec(m.lux));
            seq++;
            m.seq = seq;
            m.sender = 'Q' << 8 | read(id);
            for (uint8_t i = 3; i > 0; i--) {
                rfm.write_fsk(42, &m);
            }
            nextMeasurement.schedule();
        } else if (nextMeasurement.isNow()) {
            measure();
        } else {
            auto mode = (rfm.isIdle() && dht.isIdle() && ds.isIdle() && bh.isIdle()) ? SleepMode::POWER_DOWN
                      : SleepMode::IDLE;                    // dht is running, needs timers
            if (mode == SleepMode::POWER_DOWN && measuring) {
            	// don't power down if everything is idle here but we haven't found out yet we're done measuring
            } else {
            	//log::debug(F("States:"), '0' + rfm.isIdle(), '0' + dht.isIdle(), '0' + ds.isIdle(), '0' + bh.isIdle());
            	//log::flush();
            	power.sleepUntilAny(mode, nextMeasurement, ds, dht, bh); // TODO have dht participate in "how much time left"
            }
        }
    }

    int main() {
        while(true) {
        	loop();
        }
    }

};

}
