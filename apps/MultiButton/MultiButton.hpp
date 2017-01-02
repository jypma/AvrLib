#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "Passive/SupplyVoltage.hpp"
#include "Mechanic/Button.hpp"
#include "HopeRF/RFM12.hpp"
#include "Time/Units.hpp"
#include "Option.hpp"
#include "LED/WS2812B.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

namespace MultiButton {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace Passive;
using namespace HopeRF;
using namespace Mechanic;
using namespace LED;

struct Measurement {
	uint8_t seq;
	uint16_t sender;
	Option<uint16_t> supply;
	Option<uint8_t> button;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, Option<uint16_t>, &Measurement::supply>,
		P::Varint<10, Option<uint8_t>, &Measurement::button>
	> DefaultProtocol;
};

template <uint16_t EEPROM::*bandgapVoltage, uint8_t EEPROM::*id>
struct MultiButton {
    typedef MultiButton<bandgapVoltage,id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<100>(usart0));

    SPIMaster spi;
    ADConverter<uint16_t> adc;

    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinSupply, JeeNodePort1A());
    auto_var(pinButton1, JeeNodePort2A::withInterrupt());
    auto_var(pinButton2, JeeNodePort2D().withInterrupt());
    auto_var(pinButton3, JeeNodePortI());
    auto_var(pinLED, JeeNodePort4D());

    auto_var(rfm, (rfm12<100,100>(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz)));
    auto_var(supplyVoltage, (SupplyVoltage<4700, 1000, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(button1, Button(rt, pinButton1));
    auto_var(button2, Button(rt, pinButton2));
    auto_var(button3, Button(rt, pinButton3));
    auto_var(led, WS2812B(pinLED));

	uint8_t seq = 0;

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(power), &This::power,
            Delegate<This, decltype(button1), &This::button1,
            Delegate<This, decltype(button2), &This::button2,
            Delegate<This, decltype(button3), &This::button3
			>>>>>>> Handlers;

    static bool isDown(ButtonEvent e) {
    	return e == ButtonEvent::DOWN || e == ButtonEvent::PRESSED;
    }

    void send(bool b1, bool b2, bool b3) {
    	if (b1) {
    		led.set1({ 255, 0, 0 });
    	} else if (b2) {
    		led.set1({ 255, 255, 0 });
    	} else if (b3) {
    		led.set1({ 0, 255, 0 });
    	}

    	Measurement m = {};
    	m.seq = seq++;
        m.sender = 'B' << 8 | read(id);
        for (int i = 0; i < 10; i++) supplyVoltage.get();
        m.supply = supplyVoltage.get();
    	log::debug(F("TX "), '0' + b1, '0' + b2, '0' + b3);
    	log::debug(dec(m.supply), F("mV"));
    	log::flush();

        m.button = uint8_t(b1) | (uint8_t(b2) << 1) | (uint8_t(b3) << 2);
        rfm.write_fsk(42, &m);
    }

    void loop() {
        supplyVoltage.stopOnLowBattery(3500, [&] {
        	log::debug(F("**LOW**"));
        	log::flush();
        });

        auto b1 = button1.nextEvent();
        auto b2 = button2.nextEvent();
        auto b3 = button3.nextEvent();

    	log::debug(F("LP "), '0' + uint8_t(b1), '0' + uint8_t(b2), '0' + uint8_t(b3), F(" rfm:"), dec(uint8_t(rfm.getMode())));
    	log::flush();

        if (b1 == ButtonEvent::PRESSED || b2 == ButtonEvent::PRESSED || b3 == ButtonEvent::PRESSED) {
        	send(isDown(b1), isDown(b2), isDown(b3));
        }
        if (!isDown(b1) && !isDown(b2) && !isDown(b3)) {
        	led.set1({ 0, 0, 0});
        }

        auto rfmState = rfm.getTaskState();
        auto defaultState = TaskState(SleepMode::POWER_DOWN);
        power.sleepUntilTasks(defaultState, rfmState);
    }

    int main() {
    	rfm.onIdleSleep();
    	log::debug(F("Starting"));
        while(true) {
        	loop();
        }
    }

};

}

