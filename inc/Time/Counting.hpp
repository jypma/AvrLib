/*
 * Counting.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: jan
 */

#ifndef TIME_COUNTING_HPP_
#define TIME_COUNTING_HPP_

#include "gcc_limits.h"
#include <stdint.h>

namespace Time {

template <typename _value_t>
class Counting {
public:
    typedef _value_t value_t;

    static constexpr value_t maximum = std::numeric_limits<value_t>::max();
    /** 8 for 8-bit timer, 16 for 16-bit timer */
    static constexpr uint8_t maximumPower2 = sizeof(value_t) * 8;
};


}



#endif /* TIME_COUNTING_HPP_ */
