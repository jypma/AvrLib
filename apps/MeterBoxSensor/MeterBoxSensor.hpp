#pragma once

#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "HAL/Atmel/ADConverter.hpp"
#include "Time/RealTimer.hpp"
#include "HopeRF/RFM12.hpp"
#include "Time/Units.hpp"
#include "Option.hpp"

#define auto_var(name, expr) decltype(expr) name = expr

volatile uint8_t hcr_ints = 0;

namespace MeterBoxSensor {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace HopeRF;

struct Measurement {
	uint16_t electricity;
	uint8_t seq;
	uint16_t sender;

    typedef Protobuf::Protocol<Measurement> P;

    typedef P::Message<
		P::Varint<1, uint16_t, &Measurement::sender>,
        P::Varint<8, uint8_t, &Measurement::seq>,
		P::Varint<9, uint16_t, &Measurement::electricity>
	> DefaultProtocol;
};

template <uint8_t EEPROM::*id>
struct MeterBoxSensor {
    typedef MeterBoxSensor<id> This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<100>(usart0));

    SPIMaster spi;
    ADConverter<uint16_t> adc;

    auto_var(timer0, Timer0::withPrescaler<64>::inNormalMode());
    auto_var(timer1, Timer1::withPrescaler<1>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<64>::inNormalMode());
    auto_var(rt, realTimer(timer0));

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinElectricity, PinPD3());
    auto_var(pinElectricityValue, JeeNodePort2A());
    auto_var(pinLED, JeeNodePort2D());

    auto_var(rfm, (rfm12<4,100>(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz)));

	Measurement m = {};
	volatile uint8_t electricityCount = 0;
	uint16_t lastADC = 0;

	void onElectricityCount() {
		electricityCount++;
	}

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            	Delegate<This, decltype(rt), &This::rt,
					Delegate<This, decltype(rfm), &This::rfm,
						On<This, typename decltype(pinElectricity)::INT, &This::onElectricityCount>
    				>
    			>
    		> Handlers;

    MeterBoxSensor() {
        pinLED.configureAsOutputLow();
        log::debug(F("Starting"));
        pinTX.write(F("0000"));
        rfm.onIdleSleep();
        pinElectricity.configureAsInputWithoutPullup();
        pinElectricity.interruptOnFalling();
        log::flush();
        pinLED.setLow();
    }

    void loop() {
    	if (electricityCount > 0) {
    		log::debug(F("Counted "), dec(electricityCount));
    		{
    			AtomicScope _;
    			m.electricity += electricityCount;
    			electricityCount = 0;
    		}
    		log::debug(F("Sending "), dec(m.electricity));
            m.seq++;
            m.sender = 'R' << 8 | read(id);
            rfm.write_fsk(42, &m);
    	}
    	pinLED.setHigh(!rfm.isIdle());
    	adc.measure(pinElectricityValue);
    	uint16_t in = adc.awaitValue();
    	if ((in > lastADC && in > lastADC + 40) || (in < lastADC && in + 40 < lastADC)) {
    	    pinTX.write(dec(lastADC), F(" -> "), dec(in), endl);
    	}
    	lastADC = in;
    	pinTX.flush();
    }

    int main() {
    	while(true) {
    		loop();
    	}
    }
};

}
