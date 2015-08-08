#ifndef EEPROMTEST_H
#define EEPROMTEST_H

#include "EEPROM.hpp"

struct EEPROM {
    uint8_t data;
    char name[10];
    char apn[32];
    char password[64];
    char remoteIP[15];
    uint16_t remotePort;
};

#endif
