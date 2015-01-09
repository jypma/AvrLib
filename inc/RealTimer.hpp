/*
 * RealTimer.hpp
 *
 *  Created on: Jan 4, 2015
 *      Author: jan
 */

#ifndef REALTIMER_HPP_
#define REALTIMER_HPP_

#include "Timer.hpp"

template<typename prescaler_t>
class RealTimer {
private:
    Timer<uint8_t,prescaler_t> *timer ;
    volatile uint32_t _ticks = 0;

    void tick() {
       _ticks++;
    }

    static void doTick(volatile void *ctx) {
        ((RealTimer*)(ctx))->tick();
    }

public:
    RealTimer(Timer<uint8_t,prescaler_t> &_timer): timer(&_timer) {
        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
        timer->interruptOnOverflowOn();
    }

    RealTimer(Timer<uint8_t,prescaler_t> &_timer, TimerMode mode, prescaler_t prescaler): timer(&_timer) {
        timer->configure(mode, prescaler);

        timer->interruptOnOverflow().attach(&RealTimer::doTick, this);
        timer->interruptOnOverflowOn();
    }

    uint32_t ticks() const {
        ScopedNoInterrupts cli;
        return _ticks;
    }

    uint64_t micros() const {
        ScopedNoInterrupts cli;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)

        return (((uint64_t)_ticks) << timer->getPrescalerPower2()) / 16 * 256;
    }

    uint64_t millis() const {
        ScopedNoInterrupts cli;

#if F_CPU != 16000000
#error This function assumes 16MHz clock. Please make the function smarter if running with different clock.
#endif
        // shift left to multiply ticks by the prescaler value
        // divide by 16 ( >> 4) to go from clock ticks to microseconds
        // times 256 ( << 8) to go from clock  ticks to timer overflow (8 bit timer overflows at 256)
        // divide by 1000 to get milliseconds

        return (((uint64_t)_ticks) << timer->getPrescalerPower2()) / 16 * 256 / 1000;
    }

    void delayTicks(uint32_t ticksDelay) {
        uint32_t end = _ticks + ticksDelay;
        while (ticks() < end) ;
    }

    void delayMillis(uint16_t millisDelay) {
        uint32_t ticksDelay = (((uint32_t)millisDelay) * 1000 / 256 * 16) >> timer->getPrescalerPower2();
        delayTicks(ticksDelay);
    }
};

#endif /* REALTIMER_HPP_ */
