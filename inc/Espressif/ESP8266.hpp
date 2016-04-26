/*
 * ESP8266.hpp
 *
 *  Created on: Jun 13, 2015
 *      Author: jan
 */

#ifndef ESP8266_HPP_
#define ESP8266_HPP_

#include "ChunkedFifo.hpp"
#include "EEPROM.hpp"
#include "Streams/Scanner.hpp"
#include <util/delay.h>
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "Espressif/EthernetMACAddress.hpp"
#include "Streams/StreamingDecl.hpp"
#include "Logging.hpp"

namespace Espressif {

using namespace Time;
using namespace Streams;

template<
    char (EEPROM::*accessPoint)[32],
    char (EEPROM::*password)[64],
    char (EEPROM::*remoteIP)[15],
    uint16_t EEPROM::*remotePort,
    typename tx_pin_t,
    typename rx_pin_t,
    typename powerdown_pin_t,
    typename rt_t,
    uint8_t txFifoSize = 64,
    uint8_t rxFifoSize = 64
>
class ESP8266:
    public Streams::ReadingDelegate<ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, powerdown_pin_t, rt_t, txFifoSize, rxFifoSize>, ChunkedFifo>
{
    typedef ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, powerdown_pin_t, rt_t, txFifoSize, rxFifoSize> This;
    typedef Logging::Log<Loggers::ESP8266> log;

public:
    enum class State: uint8_t { RESYNCING, RESTARTING, DISABLING_ECHO, SETTING_STATION_MODE, GETTING_MAC_ADDRESS, LISTING_ACCESS_POINTS, CONNECTING_APN, DISABLING_MUX, CLOSING_OLD_CONNECTION, CONNECTING_UDP, CONNECTED, SENDING_LENGTH, SENDING_DATA };
private:
    Fifo<txFifoSize> txFifoData;
    ChunkedFifo txFifo = txFifoData;
    Fifo<rxFifoSize> rxFifoData;
    ChunkedFifo rxFifo = rxFifoData;
    State state = State::RESTARTING;
    tx_pin_t *tx;
    rx_pin_t *rx;
    powerdown_pin_t *pd_pin;
    VariableDeadline<rt_t> watchdog;
    uint8_t watchdogCount = 0;
    EthernetMACAddress mac;
    bool macKnown = false;

    constexpr static auto COMMAND_TIMEOUT = 1_s;
    constexpr static auto CONNECT_TIMEOUT = 10_s;
    constexpr static auto IDLE_TIMEOUT = 1_min;

    void disable_mux() {
        tx->write(F("AT+CIPMUX=0"), endl);
        state = State::DISABLING_MUX;
        watchdog.schedule(COMMAND_TIMEOUT);
    }

public:
    void resync() {
        tx->write(F("AT"), endl);
        state = State::RESYNCING;
        watchdog.schedule(COMMAND_TIMEOUT);
    }

    void restart() {
        tx->write(F("AT+RST"), endl);
        state = State::RESTARTING;
        watchdog.schedule(10_s);
    }

    EthernetMACAddress getMACAddress() {
        return mac;
    }

    bool isMACAddressKnown() {
        return macKnown;
    }
private:
    void resyncing() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                this->disable_mux();
            }
        });
    }

    void restarting() {
        scan(*rx, [this] (auto &read) {
            if (read(F("ready"))) {
                tx->write(F("ATE0"), endl);
                state = State::DISABLING_ECHO;
                watchdog.schedule(COMMAND_TIMEOUT);
            }
        });
    }

    void disabling_echo() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                tx->write(F("AT+CWMODE_CUR=1"), endl);
                state = State::SETTING_STATION_MODE;
                watchdog.schedule(COMMAND_TIMEOUT);
            }
        });
    }

    void setting_station_mode() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                tx->write(F("AT+CIPSTAMAC_CUR?"), endl);
                state = State::GETTING_MAC_ADDRESS;
                watchdog.schedule(COMMAND_TIMEOUT);
            }
        });
    }

    void getting_mac_address() {
        scan(*rx, [this] (auto &read) {
            if (read(&mac)) {
                macKnown = true;
            } else if (read(F("OK\r\n"))) {
                tx->write(F("AT+CWLAP"), endl);
                state = State::LISTING_ACCESS_POINTS;
                watchdog.schedule(CONNECT_TIMEOUT);
            }
        });
    }

    void listing_access_points() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                tx->write(F("AT+CWJAP_CUR=\""), accessPoint, F("\",\""), password, F("\""), endl);
                state = State::CONNECTING_APN;
                watchdog.schedule(CONNECT_TIMEOUT);
            }
        });
    }

    void connecting_apn() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                this->disable_mux();
            } else if (read(F("FAIL\r\n"))) {
                // Failed to connect to wifi, restart
                this->restart();
            }
        });
    }

    void disabling_mux() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                tx->write(F("AT+CIPCLOSE"), endl);
                state = State::CLOSING_OLD_CONNECTION;
                watchdog.schedule(COMMAND_TIMEOUT);
            }
        });
    }

    void closing_old_connection() {
        scan(*rx, [this] (auto &read) {
            // if deleting old conn fails, try to connect anyways.
            // Datasheet: "Prints UNLINK when there is no connection"
            if (read(F("OK\r\n")) || read(F("ERROR\r\n")) || read(F("UNLINK\r\n"))) {
                // local UDP port is always 4123
                tx->write(F("AT+CIPSTART=\"UDP\",\""), remoteIP, F("\","), dec(remotePort), F(",4123,0"), endl);
                state = State::CONNECTING_UDP;
                watchdog.schedule(COMMAND_TIMEOUT);
            }
        });
    }

    void connecting_udp() {
        scan(*rx, [this] (auto &read) {
            if (read(F("OK\r\n"))) {
                state = State::CONNECTED;
                watchdog.schedule(IDLE_TIMEOUT);
            } else if (read(F("ERROR\r\n"))) {
                // Could be "ALREADY CONNECTED"
                this->restart();
            }
        });
    }

    void connected() {
        log::debug("Scan while connected");
        scan(*rx, [this] (auto &read) {
            uint8_t length;
            rxFifo.writeStart();
            if (read(F("+IPD,"), Decimal(&length), F(":"), ChunkWithLength(&length, rxFifo))) {
                log::debug("  Got some data, %d bytes", length);
                rxFifo.writeEnd();
                watchdog.schedule(IDLE_TIMEOUT);
            } else {
                log::debug("  No data");
                rxFifo.writeAbort();
            }
        });

        if (txFifo.hasContent()) {
            log::debug("Transmitting");
            txFifo.readStart();
            auto len = txFifo.getReadAvailable();
            txFifo.readAbort();
            tx->write(F("AT+CIPSEND="), dec(len), endl);
            state = State::SENDING_LENGTH;
        }
    }

    void sending_length() {
        scan(*rx, [this] (auto &read) {
            if (read(F(">"))) {
                txFifo.readStart();
                if (tx->write(txFifo)) {
                    txFifo.readEnd();
                    state = State::SENDING_DATA;
                    watchdog.schedule(COMMAND_TIMEOUT);
                } else {
                    txFifo.readEnd(); // couldn't write, let's drop this chunk. But we're out of sync now.
                    this->restart();
                }
            }
        });
    }

    void sending_data() {
        log::debug("Scan while sending data");
        scan(*rx, [this] (auto &read) {
            uint8_t length;
            rxFifo.writeStart();
            if (read(F("+IPD,"), Decimal(&length), F(":"), ChunkWithLength(&length, rxFifo))) {
                log::debug("  Got some data");
                rxFifo.writeEnd();
                watchdog.schedule(COMMAND_TIMEOUT);
            } else if (read(F("SEND OK"))) {
                log::debug("  Got confirm");
                rxFifo.writeAbort();
                state = State::CONNECTED;
                watchdog.schedule(IDLE_TIMEOUT);
            } else if (read(F("ERROR"))) {
                log::debug("  Got error");
                rxFifo.writeAbort();
                state = State::CONNECTED;
                watchdog.schedule(IDLE_TIMEOUT);
            } else {
                log::debug("  Got nothing");
                rxFifo.writeAbort();
            }
        });
    }


public:
    ESP8266(tx_pin_t &_tx, rx_pin_t &_rx, powerdown_pin_t &_pd, rt_t &rt):
        Streams::ReadingDelegate<ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, powerdown_pin_t, rt_t, txFifoSize, rxFifoSize>, ChunkedFifo>(&rxFifo),
        tx(&_tx), rx(&_rx), pd_pin(&_pd), watchdog(rt) {
        pd_pin->configureAsOutput();
        pd_pin->setHigh();
        state = State::RESTARTING;
        watchdog.schedule(5_s);
    }

    State getState() const {
        return state;
    }

    uint8_t getWatchdogCount() const {
        return watchdogCount;
    }

    void loop() {
        if (watchdog.isNow()) {
            watchdogCount++;
            if (state == State::CONNECTED ||
                state == State::SENDING_LENGTH ||
                state == State::SENDING_DATA) {
                resync();
            } else {
                restart();
            }
        } else
        switch (state) {
        case State::RESYNCING: resyncing(); return;
        case State::RESTARTING: restarting(); return;
        case State::DISABLING_ECHO: disabling_echo(); return;
        case State::SETTING_STATION_MODE: setting_station_mode(); return;
        case State::GETTING_MAC_ADDRESS: getting_mac_address(); return;
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

    template <typename... types>
    bool write(types... args) {
        return txFifo.write(args...);
    }

    template <typename... types>
    Streams::ReadResult read(types... args) {
        return rxFifo.read(args...);
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

#include "Streams/Streaming.hpp"

#endif /* ESP8266_HPP_ */

