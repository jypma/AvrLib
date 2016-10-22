#include "HAL/Atmel/Registers.hpp"
#include <functional>
#include <iostream>

using namespace HAL::Atmel::Registers;

std::function<void()> onSleep_cpu = nullptr;

uint8_t sfr_mem[256];

uint8_t eeprom_contents[1024];

void HAL::Register8_onChange(volatile void *address) {
}

uint16_t _crc16_update(uint16_t crc, uint8_t a) {
    int i;
    crc ^= a;
    for (i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}
