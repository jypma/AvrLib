/*
 * CRC.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: jan
 */

#ifndef CRC_HPP_
#define CRC_HPP_

#include "Fifo.hpp"
#include <util/crc16.h>

class CRC16 {
    uint16_t value = ~0;
public:
    inline void reset() {
        value = ~0;
    }
    inline void append(uint8_t b) {
        value = _crc16_update(value, b);
    }
    constexpr inline uint16_t get() const {
        return value;
    }
    inline bool isValid() {
        return value == 0;
    }
};

#endif
