#include "Timers.hpp"

const TimerInfo PROGMEM timerInfos[] = {
    { (uint16_t) &TCCR0A, (uint16_t) &TCCR0B, (uint16_t) &TCNT0, (uint16_t) &TIFR0, (uint16_t) &TIMSK0 },
    { (uint16_t) &TCCR1A, (uint16_t) &TCCR1B, (uint16_t) &TCNT1, (uint16_t) &TIFR1, (uint16_t) &TIMSK1 },
    { (uint16_t) &TCCR2A, (uint16_t) &TCCR2B, (uint16_t) &TCNT2, (uint16_t) &TIFR2, (uint16_t) &TIMSK2 },
};

Timer<uint8_t> timer0(timerInfos + 0);
Timer<uint16_t> timer1(timerInfos + 1);
Timer<uint8_t> timer2(timerInfos + 2);

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
