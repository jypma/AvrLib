#ifndef GARDENVALVE_GARDENVALVE_HPP_
#define GARDENVALVE_GARDENVALVE_HPP_

#include "auto_field.hpp"
#include "HAL/Atmel/Device.hpp"
#include "HAL/Atmel/Power.hpp"
#include "Time/RealTimer.hpp"
#include "HopeRF/RFM12.hpp"
#include "Passive/SupplyVoltage.hpp"

namespace GardenValve {

using namespace HAL::Atmel;
using namespace Time;
using namespace Streams;
using namespace HopeRF;
using namespace Passive;


template <uint16_t EEPROM::*bandgapVoltage>
struct GardenValve {
    typedef GardenValve This;
    typedef Logging::Log<Loggers::Main> log;

    Usart0 usart0 = { 57600 };
    auto_var(pinTX, PinPD1<128>(usart0));

    auto_var(timer0, Timer0::withPrescaler<1024>::inNormalMode());
    auto_var(timer2, Timer2::withPrescaler<8>::inNormalMode());
    auto_var(rt, realTimer(timer0));
    auto_var(autoOff, deadline(rt, 10_min));  // Valve will stay open for []
    auto_var(autoOn, deadline(rt, 30_s));    // Valve will stay closed for [] before opening again
    auto_var(resendState, deadline(rt, 30_s));
    auto_var(resendPing, deadline(rt, 1_s));
    auto_var(deepsleep, deadline(rt, 120_min));
    auto_var(adc, ADConverter());
    bool on1 = false;

    auto_var(pinRFM12_INT, PinPD2());
    auto_var(pinRFM12_SS, PinPB2());
    auto_var(pinValve, JeeNodePort1D());
    auto_var(pinSupply, JeeNodePort1A());

    SPIMaster spi;

    auto_var(supply, (SupplyVoltage<100, 10, bandgapVoltage>(adc, pinSupply)));
    auto_var(power, Power(rt));
    auto_var(rfm, rfm12(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), RFM12Band::_868Mhz));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
            Delegate<This, decltype(rt), &This::rt,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(power), &This::power>>>> Handlers;

    uint8_t valveRemaining = 0;
    uint16_t currentSupply = 0;
    uint8_t sendAttempts = 10;

    void measureSupply() {
        supply.get();
        supply.get();
        supply.get();
        supply.get();
        currentSupply = supply.get();
    }

    void sendState() {
        measureSupply();
        rfm.write_fsk(31, 'V', '1', Padding(2),
                uint8_t(1),          // state
                uint8_t(on1 ? 1 : 0), // which valve is on
                uint8_t(valveRemaining), // Valve 1 remaining
                currentSupply);
    }

    void sendPing() {
        measureSupply();
        rfm.onIdleListen();
        log::debug(F("Ping..."));
        rfm.write_fsk(31, 'V', '1', Padding(2),
                uint8_t(0),  // ping
                uint8_t(1),  // number of valves
                currentSupply);
        resendPing.schedule();
    }

    void turnOff() {
        log::debug(F("Turning off"));
        on1 = false;
        pinValve.setLow();
        autoOff.cancel();
        rfm.onIdleSleep();
        deepsleep.schedule();
        if (valveRemaining > 1) {
            valveRemaining--;
            log::debug(F("Remaining: "), dec(valveRemaining));
            autoOn.schedule();
        } else {
            valveRemaining = 0;
        }
        sendState();
        resendState.cancel();
    }

    void turnOn() {
        log::debug(F("Turning on"));
        on1 = true;
        pinValve.setHigh();
        autoOff.schedule();
        sendState();
        resendState.schedule();
    }

public:
    int main() {
        log::debug(F("Starting"));
        measureSupply();
        measureSupply();
        measureSupply();
        measureSupply();
        measureSupply();
        autoOn.cancel();
        autoOff.cancel();
        deepsleep.cancel();
        resendState.cancel();
        pinValve.configureAsOutput();
        pinValve.setLow();
        sendPing();
        rfm.onIdleListen();
        while(true) {
            uint8_t state1 = 255;
            // We only listen to the last received packet
            while (rfm.hasContent()) {
                rfm.readStart();
                if (rfm.read(
                    Padding(3),  // header + sender
                    'V', '1',    // recipient. TODO make this in EEPROM
                    FB(2),       // expected state
                    &state1
                )) {
                }
                rfm.readEnd();
            }
            if (state1 < 10) {
                // We've received a packet
                log::debug(F("count: "), dec(state1));
                resendPing.cancel();
                rfm.onIdleSleep();
                valveRemaining = state1;
                if (valveRemaining == 0) {
                    turnOff();
                } else {
                    turnOn();
                }
            }

            if (resendPing.isNow()) {
                sendAttempts--;
                if (sendAttempts >= 0) {
                    sendPing();
                } else {
                    turnOff();
                }
            } else if (autoOff.isNow()) {
                log::debug(F("Auto-off kicking in"));
                turnOff();
            } else if (autoOn.isNow()) {
                log::debug(F("Auto-on kicking in"));
                turnOn();
            } else if (deepsleep.isNow()) {
                sendAttempts = 10;
                sendPing();
            } else if (resendState.isNow()) {
                sendState();
                resendState.schedule();
            } else {
                pinTX.flush();
                auto mode = (rfm.isSleeping() ? SleepMode::POWER_DOWN : SleepMode::IDLE);
                power.sleepUntilAny(mode, autoOff, autoOn, resendPing, resendState, deepsleep);
            }
        }
    }
};

}

#endif /* GARDENVALVE_GARDENVALVE_HPP_ */
