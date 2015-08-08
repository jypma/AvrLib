#ifndef AVR_EEPROM_H
#define AVR_EEPROM_H

extern uint8_t eeprom_contents[1024];

inline uint8_t eeprom_read_byte    (   const uint8_t *     __p )   {
    return eeprom_contents[(uint64_t) __p];
}

inline uint16_t eeprom_read_word    (   const uint16_t *     __p )   {
    return *((uint16_t*) (eeprom_contents + ((uint64_t) __p)));
}

inline uint32_t eeprom_read_dword    (   const uint32_t *     __p )   {
    return *((uint32_t*) (eeprom_contents + ((uint64_t) __p)));
}

#endif
