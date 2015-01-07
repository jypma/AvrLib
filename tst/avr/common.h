/*
 * common.h
 *
 *  Created on: Jan 6, 2015
 *      Author: jan
 */

#ifndef COMMON_H_
#define COMMON_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

extern volatile uint8_t sfr_io[256];

#define _AVR_IO_H_
#define _SFR_IO8(addr) (*(sfr_io + addr + 0x20))
#define _SFR_MEM8(addr) (*(sfr_io + addr))
#define _SFR_MEM16(addr) (*(sfr_io + addr))

#    define SREG _SFR_IO8(0x3F)

#  define SREG_C  (0)
#  define SREG_Z  (1)
#  define SREG_N  (2)
#  define SREG_V  (3)
#  define SREG_S  (4)
#  define SREG_H  (5)
#  define SREG_T  (6)
#  define SREG_I  (7)

#include "avr/iom328p.h"

#define _BV(bit) (1 << (bit))

void cli();

#define _VECTOR(idx) vector_##idx
#define SIGNAL(name) void name()

#endif /* COMMON_H_ */
