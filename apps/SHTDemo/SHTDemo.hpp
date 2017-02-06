#pragma once

#include "auto_field.hpp"
#include "HAL/Atmel/Device.hpp"
#include "Time/RealTimer.hpp"
#include "EEPROM.hpp"
#include "Strings.hpp"
#include "HAL/Atmel/TWI.hpp"
#include "Sensirion/SHT.hpp"

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Sensirion;

struct SHTDemo {
	typedef SHTDemo This;
    typedef Logging::Log<Loggers::Main> log;

    TWI<> twi = {};
    Usart0 usart0 = { 115200 };
    auto_var(pinTX, PinPD1<240>(usart0));
    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(ping, periodic(rt, 1_s));
    auto_var(sht, SHT(twi));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
    		Delegate<This, decltype(twi), &This::twi,
            Delegate<This, decltype(rt), &This::rt>>> Handlers;

    void loop() {
    	if (ping.isNow()) {
    		auto t = sht.getTemperature();
    		auto h = sht.getHumidity();
    		log::debug(F("t="), dec(t), F(" h="), dec(h));
    	}
    }

    void main() {
    	log::debug(F("SHTDemo"));
    	while(true) loop();
    }
};
