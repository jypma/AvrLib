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
};


inline void eeprom_set(uint8_t EEPROM::*field, int value) {
    eeprom->*field = value;
}

inline void eeprom_set(uint16_t EEPROM::*field, int value) {
    eeprom->*field = value;
}

template <uint8_t size>
void eeprom_set(char (EEPROM::*field)[size], const char *value) {
    const char *p = value;
    char *q = eeprom->*field;
    while (*p) {
        *q = *p;
        p++;
        q++;
    }
}


#endif
