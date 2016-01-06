#ifndef HAL_ATMEL_POWER_HPP_
#define HAL_ATMEL_POWER_HPP_

#include "AtomicScope.hpp"
#include <util/atomic.h>
#include <avr/sleep.h>

namespace HAL {
namespace Atmel {

class Power {
    uint8_t _watchdogCounter;

    void watchdogInterrupts (int8_t mode);
public:
    /**
     * Goes into sleep (power down) mode, only to be wakeable by the watchdog, hardeware, or
     * pin change interrupts.
     */
    void powerDown ();

    /**
     * Attempts to sleep (power down) for the given number of milliseconds. A hardware or pin change interrupt
     * can cause premature wakeups, but we'll continue sleeping afterwards.
     *
     * Returns whether any hardware interrupts have interrupted sleep.
     *
     * @param millisSleep Number of milliseconds to sleep at most. Must be >16, and granularity
     * is only about 16ms.
     */
    template <typename rt_t>
    bool sleepMillis(uint16_t millisSleep, rt_t &rt) {
        bool interrupted = false;
        uint16_t msleft = millisSleep;
        // only slow down for periods longer than the watchdog granularity
        while (msleft >= 16) {
            uint8_t wdp = 0; // wdp 0..9 corresponds to roughly 16..8192 ms
            // calc wdp as log2(msleft/16), i.e. loop & inc while next value is ok
            for (uint16_t m = msleft; m >= 32; m >>= 1)
                if (++wdp >= 9)
                    break;
            _watchdogCounter = 0;
            watchdogInterrupts(wdp);
            powerDown();
            watchdogInterrupts(-1); // off
            // when interrupted, our best guess is that half the time has passed
            uint16_t halfms = 8 << wdp;
            msleft -= halfms;
            if (_watchdogCounter == 0) {
                interrupted = true; // lost some time, but got interrupted
                break;
            }
            msleft -= halfms;
        }

        // adjust ticks for the delay we've just had
        millisSleep -= msleft;
        uint32_t ticksDelay = (((((uint32_t)millisSleep) * 1000) >> rt_t::maximumPower2) * 16) >> rt_t::prescalerPower2;
        { AtomicScope _;
          rt.adjustTicksForward(ticksDelay);
        }

        return interrupted;
    }

};

}
}



#endif /* HAL_ATMEL_POWER_HPP_ */
