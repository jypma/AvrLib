#include "Timers.hpp"

const TimerInfo PROGMEM timerInfos[] = {
    { (uint16_t) &TCCR0A, (uint16_t) &TCCR0B, (uint16_t) &TCNT0, (uint16_t) &TIFR0, (uint16_t) &TIMSK0, { 0, 0, 3, 6, 8, 10, 0, 0 } },
    { (uint16_t) &TCCR1A, (uint16_t) &TCCR1B, (uint16_t) &TCNT1, (uint16_t) &TIFR1, (uint16_t) &TIMSK1, { 0, 0, 3, 6, 8, 10, 0, 0 } },
    { (uint16_t) &TCCR2A, (uint16_t) &TCCR2B, (uint16_t) &TCNT2, (uint16_t) &TIFR2, (uint16_t) &TIMSK2, { 0, 0, 3, 5, 6, 7, 8, 10 } },
};

Timer<uint8_t,ExtPrescaler> timer0(timerInfos + 0);
Timer<uint16_t,ExtPrescaler> timer1(timerInfos + 1);
Timer<uint8_t,IntPrescaler> timer2(timerInfos + 2);

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

void AbstractTimer::configureFastPWM (const uint8_t prescalerBits) const {
    ScopedNoInterrupts cli;

    *regA() |= (_BV(WGM00) | _BV(WGM01));
    *regB() = (*regB() & ~_BV(WGM02) & ~_BV(CS00) & ~_BV(CS01) & ~_BV(CS02)) | prescalerBits;
}
