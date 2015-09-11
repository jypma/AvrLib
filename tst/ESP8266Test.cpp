#include "Espressif/ESP8266.hpp"
#include "EEPROMTest.hpp"
#include <Streams/Streamable.hpp>
#include <gtest/gtest.h>

namespace ESP8266Test {

using namespace Espressif;
using namespace Streams;

struct MockPin {
    bool high = false;
    void configureAsOutput() {}
    void setLow() { high = false; }
    void setHigh() { high = true; }
};

struct MockRealTimer {
    typedef uint32_t value_t;
    static constexpr uint8_t prescalerPower2 = 8;

    uint32_t count = 0;
    uint32_t counts() {
        return count;
    }
};

TEST(ESP8266Test, ESP_retries_reset_when_watchdog_fires) {
    eeprom_set(&EEPROM::apn, "apn");
    eeprom_set(&EEPROM::password, "pss");
    eeprom_set(&EEPROM::remoteIP, "host");
    eeprom_set(&EEPROM::remotePort, 123);

    Fifo<200> rx;
    Fifo<200> tx;

    MockPin reset;
    MockRealTimer rt;

    reset.high = false;
    auto esp = esp8266<&EEPROM::apn, &EEPROM::password, &EEPROM::remoteIP, &EEPROM::remotePort>(tx, rx, reset, rt);
    EXPECT_TRUE(reset.high);

    reset.high = false;
    rt.count = toCountsOn<MockRealTimer>(10000_ms) + 1000; // after timeout
    esp.loop();

    EXPECT_TRUE(reset.high);

    rx.out() << "garbage#####\r\nready\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("ATE0\r\n")>>>()));
}

TEST(ESP8266Test, ESP_can_initialize_send_and_receive) {
    eeprom_set(&EEPROM::apn, "apn");
    eeprom_set(&EEPROM::password, "pss");
    eeprom_set(&EEPROM::remoteIP, "host");
    eeprom_set(&EEPROM::remotePort, 123);

    Fifo<200> rx;
    Fifo<200> tx;

    MockPin reset;
    MockRealTimer rt;

    auto esp = esp8266<&EEPROM::apn, &EEPROM::password, &EEPROM::remoteIP, &EEPROM::remotePort>(tx, rx, reset, rt);

    EXPECT_TRUE(reset.high);

    rx.out() << "garbage#####\r\nready\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("ATE0\r\n")>>>()));

    rx.out() << "garbage###OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CWMODE_CUR=1\r\n")>>>()));

    rx.out() << "garbage##OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CWLAP\r\n")>>>()));

    rx.out() << "aplist\r\n##OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CWJAP_CUR=\"apn\",\"pss\"\r\n")>>>()));

    rx.out() << "\r\n##OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPMUX=0\r\n")>>>()));

    rx.out() << "\r\nOK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPCLOSE\r\n")>>>()));

    rx.out() << "\r\nOK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPSTART=\"UDP\",\"host\",123,4123,2\r\n")>>>()));

    rx.out() << "\r\nOK\r\n";
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

    // Send some data
    esp.out() << "hello";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPSEND=5\r\n")>>>()));

    rx.out() << "\r\nOK\r\n> ";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("hello")>>>()));

    rx.out() << "\r\nSEND OK\r\n";
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

    // Receive some data
    rx.out() << "\r\n+IPD,5:world\r\nOK";
    esp.loop();
    EXPECT_TRUE((esp.in().expect<Seq<Token<STR("world")>>>()));

    // Send some more data
    esp.out() << "cool";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPSEND=4\r\n")>>>()));

    rx.out() << "\r\nOK\r\n> ";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("cool")>>>()));

    // Pretend more received data _while_ we're still waiting on the SEND OK
    rx.out() << "\r\n+IPD,5:stuff\r\nOK";
    esp.loop();
    EXPECT_TRUE((esp.in().expect<Seq<Token<STR("stuff")>>>()));

    rx.out() << "\r\nSEND OK\r\n";
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

}

}
