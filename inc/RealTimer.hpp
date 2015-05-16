/*
 * RealTimer.hpp
 *
 *  Created on: Jan 4, 2015
 *      Author: jan
 */

#ifndef REALTIMER_HPP_
#define REALTIMER_HPP_

#include "Timer.hpp"
#include <util/atomic.h>
#include <avr/sleep.h>

inline void noop() {

}

extern uint8_t _watchdogCounter;

template<typename prescaled, prescaled *timer, uint32_t initialTicks = 0, void (*wait)() = noop>
class RealTimer {
    typedef RealTimer<prescaled,timer,initialTicks,wait> This;

    volatile uint32_t _ticks = initialTicks;

    void tick() {
       _ticks++;
    }
    InterruptHandler tickH = { this, &This::tick };

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
    RealTimer() {
        timer->interruptOnOverflow().attach(tickH);
        timer->interruptOnOverflowOn();
    }

    /**
     * Returns a 32-bit value that increments with every timer overflow.
     */
    uint32_t ticks() const {
        AtomicScope _;
        return _ticks;
    }

    /**
     * Returns a 32-bit value that increments with every timer increment.
     */
    uint32_t counts() const {
        AtomicScope _;
        return (_ticks << prescaled::maximumPower2) | timer->getValue();
    }

    uint64_t micros() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)

        return ((((uint64_t)_ticks) << prescaled::prescalerPower2) / 16) << prescaled::maximumPower2;
    }

    uint64_t millis() const {
        AtomicScope _;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)
        // divide by 1000 to get milliseconds

        return (((((uint64_t)_ticks) << prescaled::prescalerPower2) / 16) << prescaled::maximumPower2) / 1000;
    }

    void delayTicks(uint32_t ticksDelay) const {
        auto startTime = ticks();
        if (uint32_t(0xFFFFFFFF) - startTime < ticksDelay) {
            // we expect a integer wraparound.
            // first, wait for the int to overflow (with some margin)
            while (ticks() > 1) {
                wait();
            }
        }
        uint32_t end = startTime + ticksDelay;
        while (ticks() < end) {
            wait();
        }
    }

    void delayMillis(uint16_t millisDelay) const {
        uint32_t ticksDelay = (((((uint32_t)millisDelay) * 1000) >> prescaled::maximumPower2) * 16) >> prescaled::prescalerPower2;
        delayTicks(ticksDelay);
    }

    /**
     * Attempts to sleep (power down) for the given number of milliseconds. A hardware or pin change interrupt
     * can cause premature wakeups, but we'll continue sleeping afterwards.
     *
     * Returns whether any hardware interrupts have interrupted sleep.
     */
    bool sleepMillis(uint16_t millisSleep) {
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
        uint32_t ticksDelay = (((((uint32_t)millisSleep) * 1000) >> prescaled::maximumPower2) * 16) >> prescaled::prescalerPower2;
        { AtomicScope _;
          _ticks += ticksDelay;
        }

        return interrupted;
    }
};

#endif /* REALTIMER_HPP_ */
