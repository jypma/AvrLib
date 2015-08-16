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

TEST(ESP8266Test, ESP_can_be_initialized) {
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

    EXPECT_TRUE((tx.in().expect<Seq<Token<'A','T','E','0','\r','\n'>>>()));
    rx.out() << "garbage###OK\r\n";

    esp.loop();

    EXPECT_TRUE((tx.in().expect<Seq<Token<'A','T','+','C','W','M','O','D','E','_','C','U','R','=','1','\r','\n'>>>()));
    rx.out() << "garbage##OK\r\n";

    esp.loop();

    EXPECT_TRUE((tx.in().expect<Seq<Token<'A','T','+','C','W','L','A','P','\r','\n'>>>()));
    rx.out() << "aplist\r\n##OK\r\n";

    esp.loop();

    EXPECT_TRUE((tx.in().expect<Seq<Token<'A','T','+','C','W','J','A','P','_','C','U','R','=','"','a','p','n','"',',','"','p','s','s','"','\r','\n'>>>()));
    rx.out() << "\r\n##OK\r\n";

    esp.loop();

    EXPECT_TRUE((tx.in().expect<Seq<Token<'A','T','+','C','I','P','M','U','X','=','0','\r','\n'>>>()));



}

}
