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

TEST(ESP8266Test, ESP_can_initialize_send_and_receive) {
    eeprom_set(&EEPROM::apn, "apn");
    eeprom_set(&EEPROM::password, "pss");
    eeprom_set(&EEPROM::remoteIP, "host");
    eeprom_set(&EEPROM::remotePort, 123);

    Fifo<200> rx;
    Fifo<200> tx;

    MockPin reset;

    auto esp = esp8266<&EEPROM::apn, &EEPROM::password, &EEPROM::remoteIP, &EEPROM::remotePort>(tx, rx, reset);

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

    rx.out() << "\r\nSEND OK\r\n";
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

}

}
