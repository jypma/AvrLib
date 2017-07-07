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
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "Espressif/EthernetMACAddress.hpp"
#include "Streams/StreamingDecl.hpp"
#include "auto_field.hpp"
#include "HAL/Atmel/Device.hpp"
#include "Logging.hpp"

namespace Espressif {

using namespace Time;
using namespace Streams;
using namespace HAL::Atmel;

/**
 * ESP8266
 *
 * @param tx_pin_t Microcontroller pin to send to the ESP8266 (connected to RX on the ESP8266)
 * @param rx_pin_t Microcontroller pin to receive from the ESP8266 (connected to TX on the ESP8266)
 * @param powerdown_pin_t Microcontroller pin connected to PD on the ESP8266 (will power it down when low)
 */
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
class ESP8266 {
    typedef ESP8266<accessPoint, password, remoteIP, remotePort, tx_pin_t, rx_pin_t, powerdown_pin_t, rt_t, txFifoSize, rxFifoSize> This;
    typedef Logging::Log<Loggers::ESP8266> log;

public:
    enum class State: uint8_t {
        OFF_WAITING,
        ON_WAITING,
        RESYNCING,
        RESTARTING,
        DISABLING_ECHO,
        SETTING_STATION_MODE,
        GETTING_MAC_ADDRESS,
        LISTING_ACCESS_POINTS,
        CONNECTING_APN,
        DISABLING_MUX,
        CLOSING_OLD_CONNECTION,
        CONNECTING_UDP,
        CONNECTED,
        CONNECTED_DELAY,
        SENDING_LENGTH,
        SENDING_DATA };
private:
    Fifo<txFifoSize> txFifoData = {};
    ChunkedFifo txFifo = txFifoData;
    Fifo<rxFifoSize> rxFifoData = {};
    ChunkedFifo rxFifo = rxFifoData;
    State state = State::RESTARTING;
    tx_pin_t * const tx;
    rx_pin_t * const rx;
    powerdown_pin_t * const pd_pin;
    VariableDeadline<rt_t> watchdog;
    uint8_t watchdogCount = 0;
    EthernetMACAddress mac;
    bool macKnown = false;

    constexpr static auto COMMAND_TIMEOUT = 1_s;
    constexpr static auto CONNECT_TIMEOUT = 10_s;
    constexpr static auto IDLE_TIMEOUT = 5_min;
    constexpr static auto SEND_DELAY = 50_ms;
    // 1_s: no sync loss.
    // 500ms: some sync loss, not a lot. (only one in the middle)
    // 200ms: some sync loss, not a lot. (4 restarts during the night)
    // 100ms: (2 esp restarts, 2 rfm)
    // 50ms:

    constexpr static auto OFF_DELAY = 1_s;
    constexpr static auto ON_DELAY = 2_s;

    void disable_mux() {
        tx->write(F("AT+CIPMUX=0"), endl);
        state = State::DISABLING_MUX;
        watchdog.schedule(COMMAND_TIMEOUT);
    }

    void resync() {
        log::debug(F("resync"));
        tx->write(F("AT"), endl);
        state = State::RESYNCING;
        watchdog.schedule(COMMAND_TIMEOUT);
    }

    void recycle() {
        log::debug(F("recycle"));
        pd_pin->setLow();
        tx->clear();
        rx->clear();
        state = State::OFF_WAITING;
        watchdog.schedule(OFF_DELAY);
        txFifo.clear(); // power cycle is going to be too slow for these packets to matter.
    }

    void restart() {
        log::debug(F("restart"));
        tx->clear();
        rx->clear();
        tx->write(F("AT+RST"), endl);
        state = State::RESTARTING;
        watchdog.schedule(10_s);
    }

public:
    EthernetMACAddress getMACAddress() {
        return mac;
    }

    bool isMACAddressKnown() {
        return macKnown;
    }

    bool isConnected() const {
        return state == State::CONNECTED ||
               state == State::CONNECTED_DELAY ||
               state == State::SENDING_LENGTH ||
               state == State::SENDING_DATA;
    }

    bool isConnecting() const {
        return state == State::LISTING_ACCESS_POINTS ||
               state == State::CONNECTING_APN ||
               state == State::DISABLING_MUX ||
               state == State::CLOSING_OLD_CONNECTION ||
               state == State::CONNECTING_UDP;
    }

    bool isSending() const {
        return txFifo.hasContent() || state == State::SENDING_LENGTH || state == State::SENDING_DATA || state == State::CONNECTED_DELAY;
    }

    bool isReceiving() const {
        return rxFifo.hasContent();
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
                log::debug(F("ready"));
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
            EthernetMACAddress m;
            if (read(&m)) {
                mac = m;
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

    void connected(bool allowSend) {
        scan(*rx, [this] (auto &read) {
            uint8_t length;
            rxFifo.writeStart();
            if (read(F("+IPD,"), Decimal(&length), F(":"), ChunkWithLength(&length, rxFifo))) {
                //log::debug(F("in "), dec(length));
                //log::flush();
                rxFifo.writeEnd();
                /* this is not necessary.
                if (state == State::CONNECTED_DELAY) {
                    // still delaying after the previous send
                    watchdog.schedule(SEND_DELAY);
                } else {
                    watchdog.schedule(IDLE_TIMEOUT);
                }*/
                state = State::CONNECTED; // cancel any CONNECTED_DELAY, since a packet has come in, so we're good.
                watchdog.schedule(IDLE_TIMEOUT);
            } else {
                rxFifo.writeAbort();
            }
        });

        if (allowSend && txFifo.hasContent()) {
            //log::debug(F("Tx"));
            //log::flush();
            txFifo.readStart();
            auto len = txFifo.getReadAvailable();
            txFifo.readAbort();
            tx->write(F("AT+CIPSEND="), dec(len), endl);
            state = State::SENDING_LENGTH;
            watchdog.schedule(CONNECT_TIMEOUT);
        }
    }

    void sending_length() {
        scan(*rx, [this] (auto &read) {
            if (read(F(">"))) {
                txFifo.readStart();
                if (tx->write(txFifo)) {
                    txFifo.readEnd();
                    state = State::SENDING_DATA;
                    watchdog.schedule(CONNECT_TIMEOUT);
                } else {
                    txFifo.readEnd(); // couldn't write, let's drop this chunk. But we're out of sync now.
                    this->recycle();
                }
            }
        });
    }

    void sending_data() {
        scan(*rx, [this] (auto &read) {
            uint8_t length;
            rxFifo.writeStart();
            if (read(F("+IPD,"), Decimal(&length), F(":"), ChunkWithLength(&length, rxFifo))) {
                //log::debug(F("send_in"));
                rxFifo.writeEnd();
                watchdog.schedule(CONNECT_TIMEOUT);
            } else if (read(F("SEND OK"))) {
                //log::debug(F("send_ok"));
                rxFifo.writeAbort();
                state = State::CONNECTED_DELAY; // "we need to wait 20ms between packets..."
                watchdog.schedule(SEND_DELAY);
            } else if (read(F("ERROR"))) {
                //log::debug(F("send_er"));
                rxFifo.writeAbort();
                state = State::CONNECTED;
                watchdog.schedule(IDLE_TIMEOUT);
            } else if (read(F("busy"))) { // the infamous "busy s..." out-of-sync error
                this->recycle();
            } else {
                rxFifo.writeAbort();
            }
        });
    }

    void doLoop() {
        if (watchdog.isNow()) {
            if (state == State::CONNECTED_DELAY) {
                state = State::CONNECTED;
                watchdog.schedule(IDLE_TIMEOUT);
            } else if (state == State::OFF_WAITING) {
                state = State::ON_WAITING;
                pd_pin->setHigh();
                watchdog.schedule(ON_DELAY);
            } else if (state == State::ON_WAITING) {
                restart();
            } else if (state == State::CONNECTED) {
                // Just idle, normal situation.
                resync();
            } else {
                watchdogCount++;
                if (state == State::SENDING_LENGTH ||
                    state == State::SENDING_DATA) {
                    resync();
                } else if (state == State::RESTARTING) {
                    recycle();
                } else {
                    restart();
                }
            }
        } else
        switch (state) {
        case State::OFF_WAITING: return;
        case State::ON_WAITING: return;
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
        case State::CONNECTED: connected(true); return;
        case State::CONNECTED_DELAY: connected(false); return;
        case State::SENDING_LENGTH: sending_length(); return;
        case State::SENDING_DATA: sending_data(); return;
        }
    }

public:
    ESP8266(tx_pin_t &_tx, rx_pin_t &_rx, powerdown_pin_t &_pd, rt_t &rt):
        tx(&_tx), rx(&_rx), pd_pin(&_pd), watchdog(rt) {
        pd_pin->configureAsOutputLow();
        recycle();
    }

    State getState() const {
        return state;
    }

    uint8_t getWatchdogCount() const {
        return watchdogCount;
    }

    uint8_t getWatchdogCountAndReset() {
        AtomicScope _;
        const uint8_t r = watchdogCount;
        watchdogCount = 0;
        return r;
    }


    void loop() {
        const auto s = state;
        doLoop();
        if (s != state) {
            log::debug('s', dec(static_cast<uint8_t>(state)));
        }
    }

    template <typename... types>
    bool write(types... args) {
        return txFifo.write(args...);
    }

    uint8_t getSpace() const {
        return txFifo.getSpace();
    }

    inline ChunkedFifo::In in() {
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

#include "Streams/Streaming.hpp"

#endif /* ESP8266_HPP_ */

