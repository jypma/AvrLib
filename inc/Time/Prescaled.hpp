/*
 * Prescaled.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: jan
 */

#ifndef TIME_PRESCALED_HPP_
#define TIME_PRESCALED_HPP_

#include "Time/Counting.hpp"

namespace Time {

// You must define a PrescalerMeta for your prescaler_t / prescaler combination,
// with field "constexpr static uint8_t power2 = XXX", where XXX is the power of 2
// by which the CPU clock is divided, e.g. 8 for a prescaler of 256.
template<typename prescaler_t, prescaler_t prescaler>
struct PrescalerMeta {
    constexpr static uint8_t power2 = prescaler;
};

template <typename _value_t, typename _prescaler_t, _prescaler_t _prescaler>
class Prescaled: public Counting<_value_t> {
    typedef PrescalerMeta<_prescaler_t,_prescaler> Meta;
public:
    using Counting<_value_t>::maximum;
    using Counting<_value_t>::maximumPower2;

    typedef _prescaler_t prescaler_t;
    static constexpr _prescaler_t prescaler = _prescaler;
    static constexpr uint8_t prescalerPower2 = Meta::power2;
};


}



#endif /* TIME_PRESCALED_HPP_ */
