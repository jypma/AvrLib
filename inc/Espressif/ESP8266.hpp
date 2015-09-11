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
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"

namespace Espressif {

using namespace Time;
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
    typename rt_t,
    uint8_t txFifoSize = 64,
    uint8_t rxFifoSize = 64
>
class ESP8266 {
    typedef ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, rt_t, txFifoSize, rxFifoSize> This;

    template <typename... Fields> using Format = Parts::Format<This, Fields...>;
    template <typename FieldType, FieldType This::*field, class Check = void> using Scalar = Parts::Scalar<This, FieldType, field, Check>;
    template <bool (This::*condition)() const, typename... Fields> using Conditional = Parts::Conditional<This, condition, Fields...>;
    template <typename ElementType, uint8_t count, ElementType (This::*field)[count]> using Array = Parts::Array<This, ElementType, count, field>;
    template <ChunkedFifo This::*field, typename Separator = Format<This>> using Chunk = Parts::Chunk<This, field, Separator>;
public:
    enum class State: uint8_t { RESTARTING, DISABLING_ECHO, SETTING_STATION_MODE, LISTING_ACCESS_POINTS, CONNECTING_APN, DISABLING_MUX, CLOSING_OLD_CONNECTION, CONNECTING_UDP, CONNECTED, SENDING_LENGTH, SENDING_DATA };
private:
    Fifo<txFifoSize> txFifoData;
    ChunkedFifo txFifo = &txFifoData;
    Fifo<rxFifoSize> rxFifoData;
    ChunkedFifo rxFifo = &rxFifoData;
    State state = State::RESTARTING;
    tx_pin_t *tx;
    rx_pin_t *rx;
    reset_pin_t *reset_pin;
    Deadline<rt_t,decltype (10000_ms)> watchdog;

    void restart() {
        reset_pin->setLow();
        _delay_ms(1);
        reset_pin->setHigh();
        _delay_ms(1);
        reset_pin->setLow();
        _delay_ms(1);
        reset_pin->setHigh();
        state = State::RESTARTING;
        watchdog.reset();
    }

    void restarting() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("ready")>>>(s, [this] {
                tx->out() << F("ATE0") << endl;
                state = State::DISABLING_ECHO;
                watchdog.reset();
            });
        });
    }

    void disabling_echo() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                tx->out() << F("AT+CWMODE_CUR=1") << endl;
                state = State::SETTING_STATION_MODE;
                watchdog.reset();
            });
        });
    }

    void setting_station_mode() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                tx->out() << F("AT+CWLAP") << endl;
                state = State::LISTING_ACCESS_POINTS;
                watchdog.reset();
            });
        });
    }

    void listing_access_points() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                tx->out() << F("AT+CWJAP_CUR=\"") << accessPoint << F("\",\"") << password << F("\"") << endl;
                state = State::CONNECTING_APN;
                watchdog.reset();
            });
        });
    }

    void connecting_apn() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                tx->out() << F("AT+CIPMUX=0") << endl;
                state = State::DISABLING_MUX;
                watchdog.reset();
            });
            on<Format<Token<STR("FAIL\r\n")>>>(s, [this] {
                // Failed to connect to wifi, restart
                this->restart();
            });
        });
    }

    void disabling_mux() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                tx->out() << F("AT+CIPCLOSE") << endl;
                state = State::CLOSING_OLD_CONNECTION;
                watchdog.reset();
            });
        });
    }

    void closing_old_connection() {
        scan(*rx, [this] (auto s) {
            auto connect = [this] {
                // local UDP port is always 4123
                // mode 2 : any remote IP can send UDP packets to our local port 4123
                tx->out() << F("AT+CIPSTART=\"UDP\",\"") << remoteIP << F("\",") << dec(remotePort) << F(",4123,2") << endl;
                state = State::CONNECTING_UDP;
                watchdog.reset();
            };

            on<Format<Token<STR("OK\r\n")>>>(s, connect);
            on<Format<Token<STR("ERROR\r\n")>>>(s, connect);     // if deleting old conn fails, try to connect anyways.
            on<Format<Token<STR("UNLINK\r\n")>>>(s, connect);    // Datasheet: "Prints UNLINK when there is no connection"
        });
    }

    void connecting_udp() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR("OK\r\n")>>>(s, [this] {
                state = State::CONNECTED;
                watchdog.reset();
            });
            on<Format<Token<STR("ERROR\r\n")>>>(s, [this] {
                // Could be "ALREADY CONNECTED"
                this->restart();
            });
        });
    }

    template <typename scanner_t>
    inline void onReceivedData(scanner_t *s) {
        on<Format<
            Token<STR("+IPD,")>,
            Chunk<&This::rxFifo, Format<Token<STR(":")>>>>
        >(s, [this] {
            // we got some data, it's already in rxFifo.
            watchdog.reset();
        });
    }

    void connected() {
        scan(*rx, *this, [this] (auto s) {
            this->onReceivedData(s);
        });

        if (txFifo.hasContent()) {
            txFifo.readStart();
            auto len = txFifo.getReadAvailable();
            txFifo.readAbort();
            tx->out() << F("AT+CIPSEND=") << dec(len) << endl;
            state = State::SENDING_LENGTH;
        }
    }

    void sending_length() {
        scan(*rx, [this] (auto s) {
            on<Format<Token<STR(">")>>>(s, [this] {
                tx->out() << txFifo.in();
                state = State::SENDING_DATA;
                watchdog.reset();
            });
        });
    }

    void sending_data() {
        scan(*rx, *this, [this] (auto s) {
            this->onReceivedData(s);
            on<Format<Token<STR("SEND OK")>>>(s, [this] {
                state = State::CONNECTED;
                watchdog.reset();
            });
            on<Format<Token<STR("ERROR")>>>(s, [this] {
                state = State::CONNECTED;
                watchdog.reset();
            });
        });
    }


public:
    ESP8266(tx_pin_t &_tx, rx_pin_t &_rx, reset_pin_t &_reset, rt_t &rt): tx(&_tx), rx(&_rx), reset_pin(&_reset), watchdog(rt) {
        reset_pin->configureAsOutput();
        restart();
    }

    State getState() {
        return state;
    }

    void loop() {
        if (watchdog.isNow()) {
            restart();
        } else
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

    inline Reader<ChunkedFifo> in() {
        return rxFifo.in();
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
    typename reset_pin_t,
    typename rt_t
>
ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, rt_t, txFifoSize, rxFifoSize> esp8266(tx_pin_t &tx, rx_pin_t &rx, reset_pin_t &reset_pin, rt_t &rt) {
    return ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, reset_pin_t, rt_t, txFifoSize, rxFifoSize>(tx, rx, reset_pin, rt);
}

}

#endif /* ESP8266_HPP_ */

