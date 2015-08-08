#include "Espressif/ESP8266.hpp"
#include "EEPROMTest.hpp"
#include <gtest/gtest.h>

using namespace Espressif;

TEST(ESP8266Test, ESP_can_compile) {

    Fifo<200> fifo;
    Fifo<200> rx;
    Fifo<200> tx;

    ESP8266<&EEPROM::apn, &EEPROM::password, &EEPROM::remoteIP, &EEPROM::remotePort, typeof tx, typeof rx> state(tx, rx);

    state.loop();
}
