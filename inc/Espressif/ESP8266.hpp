/*
 * ESP8266.hpp
 *
 *  Created on: Jun 13, 2015
 *      Author: jan
 */

#ifndef ESP8266_HPP_
#define ESP8266_HPP_

#include "Pin.hpp"
#include "Timer.hpp"
#include "Streams/Streamable.hpp"
#include "ChunkedFifo.hpp"
#include "EEPROM.hpp"
#include "Streams/Scanner.hpp"
#include <util/delay.h>

namespace Espressif {

using namespace TimeUnits;
using namespace Streams;

// TODO add timeouts
template<
    char (EEPROM::*accessPoint)[32],
    char (EEPROM::*password)[64],
    char (EEPROM::*remoteIP)[15],
    uint16_t EEPROM::*remotePort,
    typename tx_pin_t,
    typename rx_pin_t,
    typename reset_pin_t,
    uint8_t txFifoSize = 64,
    uint8_t rxFifoSize = 64
>
class ESP8266 {
    typedef ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, txFifoSize, rxFifoSize> Type;

    template <typename... Fields> using Format = Parts::Format<Type, Fields...>;
    template <typename FieldType, FieldType Type::*field, class Check = void> using Scalar = Parts::Scalar<Type, FieldType, field, Check>;
    template <bool (Type::*condition)() const, typename... Fields> using Conditional = Parts::Conditional<Type, condition, Fields...>;
    template <typename ElementType, uint8_t count, ElementType (Type::*field)[count]> using Array = Parts::Array<Type, ElementType, count, field>;
    template <ChunkedFifo Type::*field, typename Separator = Format<Type>> using Chunk = Parts::Chunk<Type, field, Separator>;

    enum class State { RESTARTING, DISABLING_ECHO, SETTING_STATION_MODE, LISTING_ACCESS_POINTS, CONNECTING_APN, DISABLING_MUX, CLOSING_OLD_CONNECTION, CONNECTING_UDP, CONNECTED, SENDING_LENGTH, SENDING_DATA };

    Fifo<txFifoSize> txFifoData;
    ChunkedFifo txFifo = &txFifoData;
    Fifo<rxFifoSize> rxFifoData;
    ChunkedFifo rxFifo = &rxFifoData;
    State state = State::RESTARTING;
    tx_pin_t *tx;
    rx_pin_t *rx;
    reset_pin_t *reset_pin;

    void restart() {
        reset_pin->setLow();

        reset_pin->setHigh();
        _delay_ms(1);
        reset_pin->setLow();
        _delay_ms(1);
        reset_pin->setHigh();
        state = State::RESTARTING;
    }

    void restarting() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'r','e','a','d','y'>>>(s, [this] {
                tx->out() << "ATE0" << endl;
                state = State::DISABLING_ECHO;
            });
        });
    }

    void disabling_echo() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                tx->out() << "AT+CWMODE_CUR=1" << endl;
                state = State::SETTING_STATION_MODE;
            });
        });
    }

    void setting_station_mode() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                tx->out() << "AT+CWLAP" << endl;
                state = State::LISTING_ACCESS_POINTS;
            });
        });
    }

    void listing_access_points() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                tx->out() << "AT+CWJAP_CUR=\"" << accessPoint << "\",\"" << password << "\"" << endl;
                state = State::CONNECTING_APN;
            });
        });
    }

    void connecting_apn() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                tx->out() << "AT+CIPMUX=0" << endl;
                state = State::DISABLING_MUX;
            });
            on<Format<Token<'F','A','I','L','\r','n'>>>(s, [this] {
                // Failed to connect to wifi, restart
                this->restart();
            });
        });
    }

    void disabling_mux() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                tx->out() << "AT+CIPCLOSE" << endl;
                state = State::CLOSING_OLD_CONNECTION;
            });
        });
    }

    void closing_old_connection() {
        scan(*rx, [this] (auto s) {
            auto connect = [this] {
                // local UDP port is always 4123
                // mode 2 : any remote IP can send UDP packets to our local port 4123
                tx->out() << "AT+CIPSTART=\"UDP\",\"" << remoteIP << "\"," << dec(remotePort) << ",4123,2" << endl;
                state = State::CONNECTING_UDP;
            };

            on<Format<Token<'O','K','\r','\n'>>>(s, connect);
            on<Format<Token<'E','R','R','O','R','\r','\n'>>>(s, connect);     // if deleting old conn fails, try to connect anyways.
            on<Format<Token<'U','N','L','I','N','K','\r','\n'>>>(s, connect); // Datasheet: "Prints UNLINK when there is no connection"
        });
    }

    void connecting_udp() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'O','K','\r','\n'>>>(s, [this] {
                state = State::CONNECTED;
            });
            on<Format<Token<'E','R','R','O','R','\r','n'>>>(s, [this] {
                // Could be "ALREADY CONNECTED"
                this->restart();
            });
        });
    }

    void connected() {
        scan(*rx, this, [this] (auto s) {
            on<Format<
                Token<'+','I', 'P', 'D',','>,
                Chunk<&This::rxFifo, Format<Token<':'>>>>
            >(s, [this] {
                // we got some data, it's already in rxFifo.
            });
        });
        // TODO put in test that after the data, actually comes \r\nOK\r\n

        if (txFifo.hasContent()) {
            txFifo.readStart();
            auto len = txFifo.getReadAvailable();
            txFifo.readAbort();
            tx->out() << "AT+CIPSEND=" << dec(len) << endl;
            state = State::SENDING_LENGTH;
        }
    }

    void sending_length() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'>'>>>(s, [this] {
                tx->out() << txFifo.in();
                state = State::SENDING_DATA;
            });
        });
    }

    void sending_data() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<'S','E','N','D',' ','O','K'>>>(s, [this] {
                state = State::CONNECTED;
            });
            on<Format<Token<'E','R','R','O','R'>>>(s, [this] {
                state = State::CONNECTED;
            });
        });
    }


public:
    ESP8266(tx_pin_t &_tx, rx_pin_t &_rx, reset_pin_t &_reset): tx(&_tx), rx(&_rx), reset_pin(&_reset) {
        reset_pin->configureAsOutput();
        restart();
    }

    void loop() {
        switch (state) {
        case State::RESTARTING: restarting(); return;
        case State::DISABLING_ECHO: disabling_echo(); return;
        case State::SETTING_STATION_MODE: setting_station_mode(); return;
        case State::LISTING_ACCESS_POINTS: listing_access_points(); return;
        case State::CONNECTING_APN: connecting_apn(); return;
        case State::DISABLING_MUX: disabling_mux(); return;
        case State::CLOSING_OLD_CONNECTION: closing_old_connection(); return;
        case State::CONNECTING_UDP: connecting_udp(); return;
        case State::CONNECTED: connected(); return;
        case State::SENDING_LENGTH: sending_length(); return;
        case State::SENDING_DATA: sending_data(); return;
        }
    }

    inline Writer<ChunkedFifo> out() {
        return txFifo.out();
    }
};


template<
    char (EEPROM::*accessPoint)[32],
    char (EEPROM::*password)[64],
    char (EEPROM::*remoteIP)[15],
    uint16_t EEPROM::*remotePort,
    uint8_t txFifoSize = 64,
    uint8_t rxFifoSize = 64,
    typename tx_pin_t,
    typename rx_pin_t,
    typename reset_pin_t
>
ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, txFifoSize, rxFifoSize> esp8266(tx_pin_t &tx, rx_pin_t &rx, reset_pin_t &reset_pin) {
    return ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, txFifoSize, rxFifoSize>(tx, rx, reset_pin);
}

}


/*
AT+CWMODE_CUR=1

OK

AT+CWLAP

+CWLAP:(3,"AnimalZoo1",-77,"00:26:5a:d1:d7:c2",1)
+CWLAP:(0,"AnimalVisitor",-76,"06:26:5a:d1:d7:c2",1)
+CWLAP:(4,"DOVADO-be5ac",-88,"00:16:a6:1b:e5:ac",6)
+CWLAP:(3,"AnimalZoo2",-48,"90:f6:52:33:44:e0",11)
+CWLAP:(3,"TDC-2C2C",-86,"00:19:70:3d:8d:9c",11)

OK

AT+CWJAP_CUR="AnimalZoo2","**************"

AT+CIPMUX=0

OK

AT+CIPSTART="UDP","192.168.0.191",4123
0,CONNECT

OK

AT+CIPSEND=5


OK
> abcde
busy s...

SEND OK

AT+CIPCLOSE
CLOSED

OK

 */
#endif /* ESP8266_HPP_ */

