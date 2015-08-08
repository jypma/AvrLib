#include "EEPROM.hpp"
#include "avr/eeprom.h"

static constexpr EEPROM *storage = (EEPROM *) 0;

uint8_t read(uint8_t EEPROM::*field) {
    return eeprom_read_byte(&(storage->*field));
}

uint16_t read(uint16_t EEPROM::*field) {
    return eeprom_read_word(&(storage->*field));
}

uint32_t read(uint32_t EEPROM::*field) {
    return eeprom_read_dword(&(storage->*field));
}
