#include "Timer.hpp"

const TimerInfo PROGMEM timerInfos[] = {
    { &TCCR0A, &TCCR0B, &TCNT0, &TIFR0, &TIMSK0, { 0, 0, 3, 6, 8, 10, 0, 0 } },
    { &TCCR1A, &TCCR1B, &TCNT1, &TIFR1, &TIMSK1, { 0, 0, 3, 6, 8, 10, 0, 0 } },
    { &TCCR2A, &TCCR2B, &TCNT2, &TIFR2, &TIMSK2, { 0, 0, 3, 5, 6, 7, 8, 10 } },
};

Timer<uint8_t,ExtPrescaler> timer0(timerInfos + 0);
Timer<uint16_t,ExtPrescaler> timer1(timerInfos + 1);
Timer<uint8_t,IntPrescaler> timer2(timerInfos + 2);

/*
ISR(TIMER0_OVF_vect)
{
    timer0.onOverflow().invoke();
}
*/
ISR(TIMER1_OVF_vect)
{
    timer1.interruptOnOverflow().invoke();
}

ISR(TIMER2_OVF_vect)
{
    timer2.interruptOnOverflow().invoke();
}

void AbstractTimer::configureFastPWM (const uint8_t prescalerBits) const {
    sei();
    ScopedNoInterrupts cli;

    *regA() |= (_BV(WGM00) | _BV(WGM01));
    *regB() = (*regB() & ~_BV(WGM02) & ~_BV(CS00) & ~_BV(CS01) & ~_BV(CS02)) | prescalerBits;
}
