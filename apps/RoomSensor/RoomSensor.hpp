#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "HopeRF/RFM12.hpp"
#include "Dallas/DS18x20.hpp"
#include "Time/Units.hpp"
#include "DHT/DHT22.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

namespace RoomSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;
using namespace DHT;

struct Measurement {
	bool hasTemp = false;
	int16_t temp;
	uint16_t humidity;
	bool hasHumidity = false;
	uint16_t supply;
	uint8_t seq;
	uint16_t sender;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, uint16_t, &Measurement::supply>,
		P::Optional<&Measurement::hasTemp, P::Varint<10, int16_t, &Measurement::temp>>,
		P::Optional<&Measurement::hasHumidity, P::Varint<11, uint16_t, &Measurement::humidity>>
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

    auto_var(timer1, Timer1::withPrescaler<8>::inNormalMode());
    auto_var(rt, realTimer(timer1));
    auto_var(nextMeasurement, deadline(rt, 30_s));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinSupply, JeeNodePort2A());
    auto_var(pinOneWire, JeeNodePort3D());
    auto_var(pinDHT, JeeNodePort2D().withInterrupt());
    auto_var(pinDHTPower, JeeNodePort3A());

    auto_var(wire, OneWireParasitePower(pinOneWire, rt));
    auto_var(ds, SingleDS18x20(wire));
    auto_var(rfm, (rfm12<4,128>(spi, pinRFM12_SS, pinRFM12_INT, timer1.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(dht, DHT22(pinDHT, pinDHTPower, timer1.comparatorB(), rt));

	uint8_t seq = 0;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(dht), &This::dht,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(power), &This::power>>>>> Handlers;

    bool measuring = false;

    void measure() {
        log::debug(F("Measuring DS"));
        ds.measure();
        log::debug(F("Measuring DHT"));
        dht.measure();
        measuring = true;
    }

    RoomSensor() {
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
    }

    Measurement m;
    void loop() {
        supplyVoltage.stopOnLowBattery(3000);
        dht.loop();

        if (measuring && !dht.isMeasuring() && !ds.isMeasuring()) {
            pinTX.flush();
            measuring = false;
            log::debug(F("Reading DS"));
            auto t = ds.getTemperature();
            m.hasTemp = t.isDefined();
			t.forEach([this] (auto i) { m.temp = i; });
            log::debug(F("Reading Supply 1"));
            for (int i = 0; i < 10; i++) supplyVoltage.get();
            log::debug(F("Reading Supply 2"));
            m.supply = supplyVoltage.get();
            log::debug(F("Reading DHT"));

            if (dht.getLastFailure() == 0) {
            	m.humidity = dht.getHumidity();
            	m.hasHumidity = true;
            }
            log::debug(F("Humidity : "), dec(m.humidity));
            pinTX.flush();
            log::debug(F("Suppl: "), dec(m.supply));
            log::debug(F("Temp : "), dec(m.temp));
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
            auto mode = (rfm.isIdle() && dht.isIdle() && ds.isIdle()) ? SleepMode::POWER_DOWN
                      : SleepMode::IDLE;                    // dht is running, needs timers
            if (mode == SleepMode::POWER_DOWN && measuring) {
            	// don't power down if everything is idle here but we haven't found out yet we're done measuring
            } else {
            	power.sleepUntilAny(mode, nextMeasurement, ds);
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
