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
    static constexpr uint8_t maximum = 255;

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

    tx.clear();
    rt.count = uint32_t(toCountsOn<MockRealTimer>(20000_ms)) + 1000; // after timeout
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+RST\r\n")>>>()));

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

    tx.clear();
    rx.out() << "garbage#####\r\nready\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("ATE0\r\n")>>>()));

    rx.out() << "garbage###OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CWMODE_CUR=1\r\n")>>>()));

    rx.out() << "garbage##OK\r\n";
    esp.loop();
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPSTAMAC_CUR?\r\n")>>>()));
    EXPECT_FALSE(esp.isMACAddressKnown());

    esp.loop();
    rx.out() << "+";
    esp.loop();
    rx.out() << "C";
    esp.loop();
    rx.out() << "I";
    esp.loop();
    rx.out() << "P";
    esp.loop();
    rx.out() << "S";
    esp.loop();
    rx.out() << "T";
    esp.loop();
    rx.out() << "A";
    esp.loop();
    rx.out() << "M";
    esp.loop();
    rx.out() << "A";
    esp.loop();
    rx.out() << "C";
    esp.loop();
    rx.out() << "_";
    esp.loop();
    rx.out() << "C";
    esp.loop();
    rx.out() << "U";
    esp.loop();
    rx.out() << "R";
    esp.loop();
    rx.out() << ":";
    esp.loop();
    rx.out() << "\"";
    esp.loop();
    EXPECT_FALSE(esp.isMACAddressKnown());
    rx.out() << "18:";
    esp.loop();
    //rx.out() << "8";
    //esp.loop();
    //rx.out() << ":";
    esp.loop();
    rx.out() << "f";
    esp.loop();
    rx.out() << "e";
    esp.loop();
    rx.out() << ":";
    esp.loop();
    rx.out() << "3";
    esp.loop();
    rx.out() << "4";
    esp.loop();
    rx.out() << ":";
    esp.loop();
    rx.out() << "9";
    esp.loop();
    rx.out() << "f";
    esp.loop();
    rx.out() << ":";
    esp.loop();
    rx.out() << "5";
    esp.loop();
    rx.out() << "0";
    esp.loop();
    rx.out() << ":";
    esp.loop();
    rx.out() << "9";
    esp.loop();
    rx.out() << "2";
    esp.loop();
    rx.out() << "\"";
    esp.loop();
    rx.out() << "\r\n\r\nOK\r\n";
    esp.loop();



    EXPECT_TRUE(esp.isMACAddressKnown());
    EXPECT_EQ(EthernetMACAddress(0x18, 0xFE, 0x34, 0x9F, 0x50, 0x92), esp.getMACAddress());
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
    EXPECT_TRUE((tx.in().expect<Seq<Token<STR("AT+CIPSTART=\"UDP\",\"host\",123,4123,0\r\n")>>>()));

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
