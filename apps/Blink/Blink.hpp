#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "Time/Units.hpp"
#include "Option.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

namespace Blink {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;

enum State { OFF, FADEIN, ON, FADEOUT };

struct Blink {
    typedef Blink This;
    typedef Logging::Log<Loggers::Main> log;

    ADConverter adc;
    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<128>(usart0));

    auto_var(timer0, Timer0::withPrescaler<1024>::inFastPWMMode());
    auto_var(rt, realTimer(timer0));
    auto_var(nextMeasurement, deadline(rt, 5_s));
    auto_var(pinLED, PinPD5(timer0));
    auto_var(pinSupply, ArduinoPinA0());

    //auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, 1040>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(anim, animator(rt));
    auto_var(next, deadline(rt));

    State state = OFF;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt>> Handlers;

    Blink() {
        log::debug(F("Starting"));
        pinLED.configureAsOutputLow();
        pinLED.timerComparator().setOutput(FastPWMOutputMode::connected);
        pinLED.timerComparator().setTargetFromNextRun(0);
        next.schedule(500_ms);
    }

    void advance() {
    	switch(state) {
    	case OFF:
    		state = FADEIN;
    		anim.to(0xFF, 2000_ms, AnimatorInterpolation::EASE_UP);
    		break;
    	case FADEIN:
    		state = ON;
    		next.schedule(500_ms);
    		break;
    	case ON:
    		state = FADEOUT;
    		anim.to(0, 1000_ms, AnimatorInterpolation::EASE_UP);
    		break;
    	case FADEOUT:
    		state = OFF;
    		next.schedule(2500_ms);
    	}
    }

    void loop() {
    	/*
        supplyVoltage.stopOnLowBattery(3000, [&] {
        	log::debug(F("**LOW**"));
        	log::flush();
        });
        */
        log::flush();

        auto animEvt = anim.nextEvent();
        if (animEvt.isChanged()) {
        	pinLED.timerComparator().setTargetFromNextRun(animEvt.getValue());
        	if (animEvt.isIdle()) {
        		advance();
        	}
        }

        if (next.isNow()) {
        	advance();
        }
    }

    int main() {
        while(true) {
        	loop();
        }
    }

};

}
