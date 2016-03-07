#ifndef EEPROMTEST_H
#define EEPROMTEST_H

#include "EEPROM.hpp"
#include <avr/eeprom.h>

static constexpr EEPROM *eeprom = (EEPROM*) (&eeprom_contents);

struct EEPROM {
    uint8_t data;
    char name[10];
    char apn[32];
    char password[64];
    char remoteIP[15];
    uint16_t remotePort;
    uint16_t bandgapVoltage;
};


inline void eeprom_set(uint8_t EEPROM::*field, uint8_t value) {
    eeprom->*field = value;
}

inline void eeprom_set(uint16_t EEPROM::*field, uint16_t value) {
    eeprom->*field = value;
}

template <uint8_t size>
void eeprom_set(char (EEPROM::*field)[size], const char *value) {
    const char *p = value;
    char *q = eeprom->*field;
    uint8_t count = 0;
    while (*p && count < size) {
        *q = *p;
        p++;
        q++;
        count++;
    }
    if (count < size) {
        *q = 0;
    }
}


#endif
