#include "Timer.hpp"

InterruptHandler tm0int;
InterruptHandler tm1int;
InterruptHandler tm2int;

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
