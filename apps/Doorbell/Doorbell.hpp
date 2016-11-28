#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "HopeRF/RFM12.hpp"
#include "Dallas/DS18x20.hpp"
#include "Time/Units.hpp"
#include "Mechanic/Button.hpp"
#include "Option.hpp"

#pragma once

#define auto_var(name, expr) decltype(expr) name = expr

namespace Doorbell {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Dallas;
using namespace Mechanic;

struct Measurement {
	Option<int16_t> temp;
	uint16_t supply;
	uint8_t seq;
	uint16_t sender;
	Option<uint8_t> button;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, uint16_t, &Measurement::supply>,
		P::Varint<10, Option<int16_t>, &Measurement::temp>,
	    P::Varint<14, Option<uint8_t>, &Measurement::button>
	> DefaultProtocol;
};

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct Doorbell {
    typedef Doorbell<bandgapVoltage,id> This;
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
    auto_var(nextMeasurement, periodic(rt, 60_s));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinSupply, JeeNodePort2A());
    auto_var(pinOneWire, JeeNodePort1D());
    auto_var(pinButton, JeeNodePortI());

    auto_var(rfm, (rfm12<4,100>(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(wire, OneWireParasitePower(pinOneWire, rt));
    auto_var(ds, SingleDS18x20(wire));
    auto_var(button, Button(rt, pinButton));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(button), &This::button,
            Delegate<This, decltype(power), &This::power>>>>> Handlers;

	Measurement m = {};
    bool measuring = false;

    void measure() {
        log::debug(F("Measuring DS"));
        ds.measure();
        measuring = true;
    }

    Doorbell() {
        log::debug(F("Starting"));
        rfm.onIdleSleep();
        measure();
    }

    void loop() {
        supplyVoltage.stopOnLowBattery(3000);
        auto dsState = ds.getTaskState();

        if (button.nextEvent() == ButtonEvent::PRESSED) {
            log::debug(F("Button!"));
            m.button = 1;
        	m.temp = {};
        	m.supply = {};
            m.seq++;
            m.sender = 'D' << 8 | read(id);
            rfm.write_fsk(42, &m);
            log::flush();
        } else if (measuring && dsState.isIdle()) {
        	measuring = false;
        	m.button = {};
            m.temp = ds.getTemperature();
            log::debug(F("Temp : "), dec(m.temp));
            for (int i = 0; i < 10; i++) supplyVoltage.get();
            m.supply = supplyVoltage.get();
            log::debug(F("Suppl: "), dec(m.supply));
            m.seq++;
            m.sender = 'D' << 8 | read(id);
            rfm.write_fsk(42, &m);
            log::flush();
        } else if (nextMeasurement.isNow()) {
            measure();
        } else {
        	auto mode = dsState.getMaxSleepMode();
        	auto dsLeft = toMillisOn(rt, dsState.timeLeft()).getValue();
        	auto nLeft = toMillisOn(rt, nextMeasurement.timeLeft()).getValue();
        	pinTX.write('0' + dsState.isIdle(), '0' + ((uint8_t)mode), 'b', dec(button.ints), F(" ds:"), dec(dsLeft), F(" n:"), dec(nLeft), endl);
        	pinTX.flush();
        	power.sleepUntilAny(mode, nextMeasurement, dsState);
        }
    }

    void main() {
    	while(true) {
    		loop();
    	}
    }
};

}

