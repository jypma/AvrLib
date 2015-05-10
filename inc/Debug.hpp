/*
 * Debug.hpp
 *
 *  Created on: May 8, 2015
 *      Author: jan
 */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <avr/io.h>

extern uint16_t debugTimings[256];
extern uint8_t debugTimingsCount;

static uint16_t debugStartTime;

inline void debugTimeStart() {
    debugStartTime = TCNT1;
}

inline void debugTimeEnd() {
    uint16_t duration = TCNT1 - debugStartTime;

    if (debugTimingsCount < 254) {
        debugTimings[debugTimingsCount] = duration;
        debugTimingsCount++;
    }
}


#endif /* DEBUG_HPP_ */
