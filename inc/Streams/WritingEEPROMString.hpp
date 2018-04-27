#ifndef STREAMS_WRITINGEEPROMSTRING_HPP_
#define STREAMS_WRITINGEEPROMSTRING_HPP_

#include <stdint.h>
#define _AVR_IO_H_
#define E2END 0x3FF // anything
#include <avr/eeprom.h>
#undef E2END
#undef _AVR_IO_H_

namespace Streams {
namespace Impl {

template <typename sem, typename fifo_t>
bool writeStringFromEEPROM(fifo_t &fifo, uint8_t *pos, uint8_t maxLength) {
    while (maxLength > 0) {
        uint8_t value = eeprom_read_byte(pos);
        if (value == 0) {
            return true;
        }
        if (sem::canWrite(fifo, 1)) {
            sem::write(fifo, value);
        } else {
            return false;
        }
        pos++;
        maxLength--;
    }
    return true;
}

template <typename sem, typename fifo_t, uint8_t size>
bool write1(fifo_t &fifo, char (EEPROM::*field)[size]) {
    return writeStringFromEEPROM<sem>(fifo, (uint8_t *) (((EEPROM*)0)->*field), size);
}



}
}


#endif /* STREAMS_WRITINGEEPROMSTRING_HPP_ */
