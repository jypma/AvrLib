#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "Time/Units.hpp"
#include "Option.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

namespace ADCCalibrate {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;

template <uint16_t EEPROM::*bandgapVoltage>
struct ADCCalibrate {
	typedef ADCCalibrate This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<100>(usart0));

    ADConverter<uint16_t> adc;
    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(pinSupply, JeeNodePort2A());
    auto_var(go, periodic(rt, 200_ms));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt>> Handlers;

    ADCCalibrate() {
    	pinSupply.configureAsInputWithoutPullup();
    }

    void loop() {
    	if (go.isNow()) {
    		uint16_t v = supplyVoltage.get();
    		log::debug(dec(v), F("mV"));
    		log::flush();
    	}
    }

    int main() {
    	log::debug(F("ADCCalibrate"));
    	while(true) {
    		loop();
    	}
    }
};

}

