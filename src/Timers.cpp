#include "Timers.hpp"

const TimerInfo PROGMEM timerInfos[] = {
    { (uint16_t) &TCCR0A, (uint16_t) &TCCR0B, (uint16_t) &TCNT0, (uint16_t) &TIFR0, (uint16_t) &TIMSK0 },
    { (uint16_t) &TCCR1A, (uint16_t) &TCCR1B, (uint16_t) &TCNT1, (uint16_t) &TIFR1, (uint16_t) &TIMSK1 },
    { (uint16_t) &TCCR2A, (uint16_t) &TCCR2B, (uint16_t) &TCNT2, (uint16_t) &TIFR2, (uint16_t) &TIMSK2 },
};

Timer8 timer0(timerInfos + 0);
Timer16 timer1(timerInfos + 1);
Timer8 timer2(timerInfos + 2);

/*
SIGNAL(TIMER0_OVF_vect)
{
    timer0.onOverflow().invoke();
}
*/
SIGNAL(TIMER1_OVF_vect)
{
    timer1.interruptOnOverflow().invoke();
}

SIGNAL(TIMER2_OVF_vect)
{
    timer2.interruptOnOverflow().invoke();
}

void Timer::configure (const TimerSettings &settings) const {
    ScopedNoInterrupts cli;

    switch (settings.mode) {
        case TimerMode::fastPWM:
            *regA() |= (_BV(WGM00) | _BV(WGM01));
            *regB() &= ~_BV(WGM02);
            break;
    }

    *regB() = (*regB() & ~(_BV(CS00) | _BV(CS01) | _BV(CS02))) | static_cast<uint8_t>(settings.prescaler);
}

Timer& Timer::configureFastPWM(uint8_t on, uint8_t off) const {
    ScopedNoInterrupts cli;

    *regA() |= (_BV(WGM00) | _BV(WGM01));
    *regB() |= on;
    *regB() &= ~(_BV(WGM02) | off);
    return (Timer&) *this;
}

volatile void *lastCtx = nullptr;
int timers = 0;

uint16_t Timer::getPrescaler() const {
    switch(*regB() & (_BV(CS00) | _BV(CS01) | _BV(CS02))) {
        case _BV(CS00): return 1;
        case _BV(CS01): return 8;
        case _BV(CS01) | _BV(CS00): return 64;
        case _BV(CS02): return 256;
        case _BV(CS02) | _BV(CS00): return 1024;
        default: return 1024;
    }
}

void RealTimer::tick() {
    /*
   uint64_t m = _millis;
   uint16_t f = _fract;

   m += millisInc;
   f += fractInc;

   if (f >= 1000) {
       f -= 1000;
       m += 1;
   }

   _fract = f;
   _millis = m;
   */
   _ticks++;
}
