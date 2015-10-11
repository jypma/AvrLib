#ifndef HAL_ATMEL_POWER_HPP_
#define HAL_ATMEL_POWER_HPP_

#include "AtomicScope.hpp"
#include <util/atomic.h>
#include <avr/sleep.h>

class Power {
    uint8_t _watchdogCounter;

    void watchdogInterrupts (uint8_t mode) const {
        // correct for the fact that WDP3 is *not* in bit position 3!
        if (mode & _BV(3))
            mode ^= _BV(3) | _BV(WDP3);
        // pre-calculate the WDTCSR value, can't do it inside the timed sequence
        // we only generate interrupts, no reset
        uint8_t wdtcsr = mode >= 0 ? _BV(WDIE) | mode : 0;
        MCUSR &= ~(1<<WDRF);
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
    #ifndef WDTCSR
    #define WDTCSR WDTCR
    #endif
            WDTCSR |= (1<<WDCE) | (1<<WDE); // timed sequence
            WDTCSR = wdtcsr;
        }
    }

    void powerDown () const {
        uint8_t adcsraSave = ADCSRA;
        ADCSRA &= ~ _BV(ADEN); // disable the ADC
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
            sleep_enable();
            sleep_bod_disable(); // can't use this - not in my avr-libc version!
    //#ifdef BODSE
    //        MCUCR = MCUCR | _BV(BODSE) | _BV(BODS); // timed sequence
    //        MCUCR = (MCUCR & ~ _BV(BODSE)) | _BV(BODS);
    #//endif
        }
        sleep_cpu();
        sleep_disable();
        // re-enable what we disabled
        ADCSRA = adcsraSave;
    }

public:
    /**
     * Attempts to sleep (power down) for the given number of milliseconds. A hardware or pin change interrupt
     * can cause premature wakeups, but we'll continue sleeping afterwards.
     *
     * Returns whether any hardware interrupts have interrupted sleep.
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
        uint32_t ticksDelay = (((((uint32_t)millisSleep) * 1000) >> timer_t::maximumPower2) * 16) >> timer_t::prescalerPower2;
        { AtomicScope _;
          _ticks += ticksDelay;
        }

        return interrupted;
    }

};



#endif /* HAL_ATMEL_POWER_HPP_ */
