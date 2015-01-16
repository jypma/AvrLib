#include "Timer.hpp"

TimerInterruptHandler tm0int;
TimerInterruptHandler tm1int;
TimerInterruptHandler tm2int;

ISR(TIMER0_OVF_vect)
{
    tm0int.invoke();
}

ISR(TIMER1_OVF_vect)
{
    tm1int.invoke();
}

ISR(TIMER2_OVF_vect)
{
    tm2int.invoke();
}
