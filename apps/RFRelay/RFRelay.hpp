#ifndef RFRELAY_RFRELAY_HPP_
#define RFRELAY_RFRELAY_HPP_

#include "auto_field.hpp"
#include "HAL/Atmel/Device.hpp"
#include "Espressif/ESP8266.hpp"
#include "Time/RealTimer.hpp"
#include "EEPROM.hpp"
#include "Strings.hpp"
#include "Serial/PulseCounter.hpp"
#include "FS20/FS20Decoder.hpp"
#include "Visonic/VisonicDecoder.hpp"
#include "HopeRF/RFM12.hpp"

namespace RFRelay {

using namespace HAL::Atmel;
using namespace Espressif;
using namespace Time;
using namespace FS20;
using namespace Visonic;
using namespace Serial;
using namespace HopeRF;
using namespace Streams;

template <
	char (EEPROM::*apn)[32],
	char (EEPROM::*password)[64],
	char (EEPROM::*remoteIP)[15],
	uint16_t EEPROM::*remotePort>
struct RFRelay {
    typedef RFRelay<apn,password,remoteIP,remotePort> This;

    SPIMaster spi;
    static auto constexpr band = RFM12Band::_868Mhz;

    Usart0 usart0 = { 115200 };
    let(pinTX, PinPD1<128>(usart0));
    let(pinRX, PinPD0<128>(usart0));

    let(timer0, Timer0::withPrescaler<64>::inNormalMode());
    let(rt, realTimer(timer0));
    let(pinRFM12_INT, PinPD2());
    let(pinOOK, PinPD3());
    let(pinOOK_EN, PinPD4());
    let(pinLED0, PinPD5());
    let(pinLED1, PinPD6());
    let(pinESP_PD, PinPD7());
    let(pinLED2, PinPB0());
    let(pinLED3, PinPB1());
    let(pinRFM12_SS, PinPB2());
    let(rfm, rfm12(spi, pinRFM12_SS, pinRFM12_INT, timer0.comparatorA(), band));
    let(counter, pulseCounter(timer0.comparatorB(), pinOOK, 150_us));
    let(esp, (esp8266<apn,password,remoteIP,remotePort>(pinTX, pinRX, pinESP_PD, rt)));
    let(fs20, fs20Decoder(counter));
    let(visonic, visonicDecoder(counter));
    let(pingInterval, periodic(rt, 30_s));
    let(rfmWatchdog, deadline(rt, 60000_ms));

    typedef Delegate<This, decltype(pinTX), &This::pinTX,
    		Delegate<This, decltype(pinRX), &This::pinRX,
            Delegate<This, decltype(rt), &This::rt,
            Delegate<This, decltype(rfm), &This::rfm,
            Delegate<This, decltype(counter), &This::counter>>>>> Handlers;

    uint8_t rfmWatchdogCount = 0;
    uint8_t packets = 0;
    let(blink, deadline(rt));
    bool blinkOn = false;
    uint8_t blinkIdx = 0;

    typedef uint8_t PacketType;
    static constexpr PacketType TYPE_RFM12 = 'R';
    static constexpr PacketType TYPE_FS20 = 'F';
	static constexpr PacketType TYPE_VISONIC = 'V';
	static constexpr PacketType TYPE_PING = 'P';
	static constexpr PacketType TYPE_MSG = 'M';

    void send() {
        static uint8_t count = 0;

        FS20Packet fs20Packet;
        if (fs20.read(&fs20Packet)) {
            esp.write(TYPE_FS20, &fs20Packet);
        }

        VisonicPacket visonicPacket;
        if (visonic.read(&visonicPacket)) {
            esp.write(TYPE_VISONIC, &visonicPacket);
        }
        if (rfm.hasContent()) {
            rfmWatchdog.schedule();
            rfm.readStart();
            esp.write(TYPE_RFM12, rfm);
            rfm.readEnd();
        }
        if (pingInterval.isNow()) {
            count++;
            // TODO include here: OOK overflows, ESP watchdog fires, NODE ID (4 bytes)
            // TODO describe protocol in a struct, properly.
            auto mac = esp.getMACAddress();
            esp.write(TYPE_PING, &mac, ',', dec(count), ',', dec(packets), ',', dec(esp.getWatchdogCount()), ',', dec(rfmWatchdogCount));
        }
    }

    RFRelay() {
        blink.schedule(400_ms);
        pinOOK.configureAsInputWithPullup();
        pinOOK_EN.configureAsOutput();
        pinOOK_EN.setHigh();
        pinLED0.configureAsOutput();
        pinLED1.configureAsOutput();
        pinLED2.configureAsOutput();
        pinLED3.configureAsOutput();
    }

    void loop() {
        if (blink.isNow()) {
            blinkOn = !blinkOn;
            if (blinkOn) {
                blink.schedule(100_ms);
            } else {
                blink.schedule(400_ms);
                blinkIdx = blinkIdx + 1;
                if (blinkIdx > 3) {
                    blinkIdx = 0;
                }
            }
        }

        uint8_t state = static_cast<uint8_t>(esp.getState());
        if (blinkOn) {
            state ^= (1 << blinkIdx);
        }
        pinLED0.setHigh(state & 1);
        pinLED1.setHigh(state & 2);
        pinLED2.setHigh(state & 4);
        pinLED3.setHigh(state & 8);
        counter.onMax(10, [&] (Pulse pulse) {
            fs20.apply(pulse);
            visonic.apply(pulse);
        });

        esp.loop();

        if (esp.isMACAddressKnown()) {
            send();
        }

        if (rfmWatchdog.isNow()) {
            rfmWatchdogCount++;
            rfm.reset(band);
            rfmWatchdog.schedule();
        }

        esp.read(Nested([&] (auto read) -> ReadResult {
            uint8_t type;

            if (!read(&type)) return ReadResult::Invalid;
            switch(type) {
            case TYPE_FS20: {
                FS20Packet inPacket;
                if (!read(&inPacket)) return ReadResult::Invalid;
				rfm.write_fs20(inPacket);
				packets++;
				break;
            }
            case TYPE_RFM12: {
            	uint8_t header;
            	if (!read(&header)) return ReadResult::Invalid;
            	rfm.write_fsk(header, read);
            	packets++;
            }
            }
            return ReadResult::Valid;
        }));
    }

    int main() {
        while(true) {
        	loop();
        }
    }
};

}



#endif /* RFRELAY_RFRELAY_HPP_ */
