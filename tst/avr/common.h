/*
 * common.h
 *
 *  Created on: Jan 6, 2015
 *      Author: jan
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define _AVR_IO_H_
#define __AVR_ATmega328P__

extern uint8_t sfr_mem[256];

#define _SFR_IO8(addr) sfr_mem[addr +  + 0x20]
#define _SFR_MEM8(addr) sfr_mem[addr]
#define _SFR_MEM16(addr) *((uint16_t*)(&sfr_mem[addr]))

#define F_CPU 16000000

#    define SREG _SFR_IO8(0x3F)

#  define SREG_C  (0)
#  define SREG_Z  (1)
#  define SREG_N  (2)
#  define SREG_V  (3)
#  define SREG_S  (4)
#  define SREG_H  (5)
#  define SREG_T  (6)
#  define SREG_I  (7)

#define _VECTOR(idx) vector_##idx
#define ISR(name) void name ()

#define _BV(bit) (1 << (bit))

void cli();
void sei();

#endif /* COMMON_H_ */
