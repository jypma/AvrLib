/*
 * EEPROM.hpp
 *
 *  Created on: Jul 25, 2015
 *      Author: jan
 */

#ifndef EEPROM_HPP_
#define EEPROM_HPP_

#include <stdint.h>

struct EEPROM;

uint8_t read(uint8_t EEPROM::*field);
uint16_t read(uint16_t EEPROM::*field);
uint32_t read(uint32_t EEPROM::*field);

#endif /* EEPROM_HPP_ */
