/*
 * pgmspace.h
 *
 *  Created on: Jan 6, 2015
 *      Author: jan
 */

#ifndef PGMSPACE_H_
#define PGMSPACE_H_

#define PROGMEM

#define pgm_read_ptr(ptr) ((void *)ptr)

#define pgm_read_byte(ptr) (*((uint8_t *)ptr))



#endif /* PGMSPACE_H_ */
