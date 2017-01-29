#pragma once

#include "auto_field.hpp"
#include "HAL/Atmel/Device.hpp"
#include "Time/RealTimer.hpp"
#include "EEPROM.hpp"
#include "Strings.hpp"
#include "Serial/RS232Tx.hpp"

using namespace HAL::Atmel;
using namespace Time;
using namespace Serial;
using namespace Streams;
using namespace Serial;

struct RS232TxTest {
	typedef RS232TxTest This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 115200 };
    auto_var(pinTX, PinPD1<240>(usart0));
    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(timer1, Timer1::withPrescaler<1>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(pinLogTX, PinPD5(timer0));
    auto_var(logTX, RS232Tx<>(pinLogTX));
    auto_var(ping, periodic(rt, 1_s));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(logTX), &This::logTX>>> Handlers;

    void loop() {
    	log::flush();
    	Logging::printTimings();
    	log::flush();
    	if (ping.isNow()) {
    		log::debug(F("Go! "), dec(debugTimingsCount));
            logTX.write(F("eeeee"));
    	}
    }

    void main() {
    	while(true) loop();
    }
};
