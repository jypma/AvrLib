#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "DHT/DHT22.hpp"
#include "DHT/DHT11.hpp"

// Timing: 120,119,120,120,120,119,120,120,120

#define auto_var(name, expr) decltype(expr) name = expr

namespace DHTCalib {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace DHT;

struct DHTCalib {
    typedef DHTCalib This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<250>(usart0));
    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(timer1, Timer1::withPrescaler<64>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(nextMeasurement, deadline(rt, 2_s));

    auto_var(pinDHTPower, ArduinoPinD4());
    auto_var(pinDHT0, ArduinoPinA1().withInterruptOnChange());
    auto_var(pinDHT1, ArduinoPinA2().withInterruptOnChange());
    auto_var(pinDHT2, ArduinoPinA3().withInterruptOnChange());
    //auto_var(pinDHT3, ArduinoPinA3().withInterrupt());
    //auto_var(pinDHT4, ArduinoPinD13().withInterrupt());
    //auto_var(pinDHT5, ArduinoPinD12().withInterrupt());

    auto_var(dht0, DHT22(pinDHT0, pinDHTPower, timer0.comparatorA(), rt));
    auto_var(dht1, DHT22(pinDHT1, pinDHTPower, timer0.comparatorB(), rt));
    auto_var(dht2, DHT11(pinDHT2, pinDHTPower, timer1.comparatorA(), rt));
    //auto_var(dht3, DHT11(pinDHT3, pinDHTPower, timer1.comparatorB(), rt));
    //auto_var(dht4, DHT11(pinDHT4, pinDHTPower, timer2.comparatorA(), rt));
    //auto_var(dht5, DHT11(pinDHT5, pinDHTPower, timer2.comparatorB(), rt));

    /*
    volatile uint8_t changes = 0;
    void onChange() {
    	changes++;
    }
    */

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
			Delegate<This, decltype(dht0), &This::dht0,
			Delegate<This, decltype(dht1), &This::dht1,
			Delegate<This, decltype(dht2), &This::dht2
			>>>>> Handlers;

    void measure() {
        log::debug(F("Measuring DHT0"));
        dht0.measure();
        while (dht0.isMeasuring()) {
        //	Logging::printTimings();
        //	log::flush();
        	dht0.loop();
        }

        log::debug(F("Measuring DHT1"));
        dht1.measure();
        while (dht1.isMeasuring()) {
        	//log::debug(dec(pls));
        	//log::debug(dec(HAL::Atmel::PinChangeVectorOnChange<HAL::Atmel::Info::PCInt1Info, 1 << 0>::ints));
        	//log::debug(dec(HAL::Atmel::PinChangeVectorOnChange<HAL::Atmel::Info::PCInt1Info, 1 << 1>::ints));
        	//log::debug(dec(HAL::Atmel::PinChangeVectorOnChange<HAL::Atmel::Info::PCInt1Info, 1 << 2>::ints));
        	//log::flush();
            //Logging::printTimings();
        	dht1.loop();
        }
    }

    DHTCalib() {
    	//pinDHT1.configureAsInputWithoutPullup();
    	//pinDHT1.interruptOnChange();
        log::debug(F("Starting"));
        uint16_t t;
        {
        	AtomicScope _;
        	t = TCNT1;
        	Logging::TimingEnabled::timeStart();
        	Logging::TimingEnabled::timeEnd();
        	t = TCNT1 - t;
        }
        log::debug(F("Timing overhead: "), dec(t));
    }

    void loop() {
    	//pinTX.write(dec(pls), '/', dec(debugTimingsCount), endl);
        dht0.loop();
        dht1.loop();
        //dht2.loop();
        //dht3.loop();
        //dht4.loop();
        //dht5.loop();
        pinTX.flush();
        //log::debug(F("changes: "), dec(changes), F(", high: "), '0' + pinDHT1.isHigh(), F(", PCICR="), dec(PCICR));
        Logging::printTimings();
        pinTX.flush();


        if (nextMeasurement.isNow()) {
            measure();
            log::debug(F("DHT0: h="), dec(dht0.getHumidity()), F(" t="), dec(dht0.getTemperature()));
            log::debug(F("DHT1: h="), dec(dht1.getHumidity()), F(" t="), dec(dht1.getTemperature()));
        //    pinTX.flush();
            nextMeasurement.schedule();
        }

    }

    int main() {
        while(true) {
        	loop();
        }
    }
};

}
