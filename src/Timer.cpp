#include "Timer.hpp"

InterruptHandler tm0int;
InterruptHandler tm0ocra;
InterruptHandler tm0ocrb;
InterruptHandler tm1int;
InterruptHandler tm1ocra;
InterruptHandler tm1ocrb;
InterruptHandler tm2int;
InterruptHandler tm2ocra;
InterruptHandler tm2ocrb;

ISR(TIMER0_OVF_vect)
{
    tm0int.invoke();
}

ISR(TIMER0_COMPA_vect)
{
    tm0ocra.invoke();
}

ISR(TIMER0_COMPB_vect)
{
    tm0ocrb.invoke();
}

ISR(TIMER1_OVF_vect)
{
    tm1int.invoke();
}

ISR(TIMER1_COMPA_vect)
{
    tm1ocra.invoke();
}

ISR(TIMER1_COMPB_vect)
{
    tm1ocrb.invoke();
}

ISR(TIMER2_OVF_vect)
{
    tm2int.invoke();
}

ISR(TIMER2_COMPA_vect)
{
    tm2ocra.invoke();
}

ISR(TIMER2_COMPB_vect)
{
    tm2ocrb.invoke();
}

