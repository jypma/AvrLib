#include "Timers.hpp"

Timer<uint8_t> timer0(timerInfos + 0);
Timer<uint16_t> timer1(timerInfos + 1);
Timer<uint8_t> timer2(timerInfos + 2);

SIGNAL(TIMER0_OVF_vect)
{
    if (timer0.onOverflow != nullptr) {
        timer0.onOverflow(timer0.onOverflowCtx);
    }
}

SIGNAL(TIMER1_OVF_vect)
{
    if (timer1.onOverflow != nullptr) {
        timer1.onOverflow(timer0.onOverflowCtx);
    }
}

SIGNAL(TIMER2_OVF_vect)
{
    if (timer2.onOverflow != nullptr) {
        timer2.onOverflow(timer0.onOverflowCtx);
    }
}
