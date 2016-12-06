#include "Espressif/ESP8266.hpp"
#include "EEPROMTest.hpp"
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
    rt.count = uint32_t(toCountsOn<MockRealTimer>(20000_ms).getValue()) + 1000; // after timeout
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+RST\r\n")));

    rx.write(F("garbage#####\r\nready\r\n"));
    esp.loop();

    EXPECT_TRUE(tx.read(F("ATE0\r\n")));
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
    rx.write(F("garbage#####\r\nready\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("ATE0\r\n")));

    rx.write(F("garbage###OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CWMODE_CUR=1\r\n")));

    rx.write(F("garbage##OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPSTAMAC_CUR?\r\n")));
    EXPECT_FALSE(esp.isMACAddressKnown());

    esp.loop();
    rx.write(F("+"));
    esp.loop();
    rx.write(F("C"));
    esp.loop();
    rx.write(F("I"));
    esp.loop();
    rx.write(F("P"));
    esp.loop();
    rx.write(F("S"));
    esp.loop();
    rx.write(F("T"));
    esp.loop();
    rx.write(F("A"));
    esp.loop();
    rx.write(F("M"));
    esp.loop();
    rx.write(F("A"));
    esp.loop();
    rx.write(F("C"));
    esp.loop();
    rx.write(F("_"));
    esp.loop();
    rx.write(F("C"));
    esp.loop();
    rx.write(F("U"));
    esp.loop();
    rx.write(F("R"));
    esp.loop();
    rx.write(F(":"));
    esp.loop();
    rx.write(F("\""));
    esp.loop();
    EXPECT_FALSE(esp.isMACAddressKnown());
    rx.write(F("18:"));
    esp.loop();
    //rx.write(F("8"));
    //esp.loop();
    //rx.write(F(":"));
    esp.loop();
    rx.write(F("f"));
    esp.loop();
    rx.write(F("e"));
    esp.loop();
    rx.write(F(":"));
    esp.loop();
    rx.write(F("3"));
    esp.loop();
    rx.write(F("4"));
    esp.loop();
    rx.write(F(":"));
    esp.loop();
    rx.write(F("9"));
    esp.loop();
    rx.write(F("f"));
    esp.loop();
    rx.write(F(":"));
    esp.loop();
    rx.write(F("5"));
    esp.loop();
    rx.write(F("0"));
    esp.loop();
    rx.write(F(":"));
    esp.loop();
    rx.write(F("9"));
    esp.loop();
    rx.write(F("2"));
    esp.loop();
    rx.write(F("\""));
    esp.loop();
    rx.write(F("\r\n\r\nOK\r\n"));
    esp.loop();



    EXPECT_TRUE(esp.isMACAddressKnown());
    EXPECT_EQ(EthernetMACAddress(0x18, 0xFE, 0x34, 0x9F, 0x50, 0x92), esp.getMACAddress());
    EXPECT_TRUE(tx.read(F("AT+CWLAP\r\n")));

    rx.write(F("aplist\r\n##OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CWJAP_CUR=\"apn\",\"pss\"\r\n")));

    rx.write(F("\r\n##OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPMUX=0\r\n")));

    rx.write(F("\r\nOK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPCLOSE\r\n")));

    rx.write(F("\r\nOK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPSTART=\"UDP\",\"host\",123,4123,0\r\n")));

    rx.write(F("\r\nOK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

    // Send some data
    esp.write(F("hello"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPSEND=5\r\n")));

    rx.write(F("\r\nOK\r\n> "));
    esp.loop();
    EXPECT_TRUE(tx.read(F("hello")));

    rx.write(F("\r\nSEND OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

    // Receive some data
    rx.write(F("\r\n+IPD,5:world\r\nOK"));
    esp.loop();
    EXPECT_TRUE(esp.read(F("world")));

    // Send some more data
    esp.write(F("cool"));
    esp.loop();
    EXPECT_TRUE(tx.read(F("AT+CIPSEND=4\r\n")));

    rx.write(F("\r\nOK\r\n> "));
    esp.loop();
    EXPECT_TRUE(tx.read(F("cool")));

    // Pretend more received data _while_ we're still waiting on the SEND OK
    rx.write(F("\r\n+IPD,5:stuff\r\nOK"));
    esp.loop();
    EXPECT_TRUE(esp.read(F("stuff")));

    rx.write(F("\r\nSEND OK\r\n"));
    esp.loop();
    EXPECT_TRUE(tx.isEmpty());

}

TEST(ESP8266Test, can_parse_mac_address) {
    auto fifo = Fifo<24>();
    fifo.write(F("06:0f:A0:de:21:4f"));
    EthernetMACAddress address;
    EXPECT_TRUE(fifo.read(&address));
    EXPECT_EQ(0x06, address.byte1());
    EXPECT_EQ(0x0F, address.byte2());
    EXPECT_EQ(0xA0, address.byte3());
    EXPECT_EQ(0xDE, address.byte4());
    EXPECT_EQ(0x21, address.byte5());
    EXPECT_EQ(0x4F, address.byte6());
}

TEST(ESP8266Test, can_write_mac_address) {
    EthernetMACAddress address = { 0xF0, 0x34, 0x02, 0x4A, 0xDE, 0x00 };
    auto fifo = Fifo<24>();
    fifo.write(&address);
    EXPECT_TRUE(fifo.read(F("F0:34:02:4A:DE:00")));
}

}
