#include "HAL/Atmel/Power.hpp"

using namespace HAL::Atmel;
using namespace HAL::Atmel::Registers;

#ifdef AVR
INLINE static void sleep_cpu() {
    do {
        __asm__ __volatile__ ( "sleep" "\n\t" :: );
    } while(0);
}
#else
INLINE static void sleep_cpu() {
    if (onSleep_cpu != nullptr) {
        onSleep_cpu();
    }
}
#endif

INLINE static void sleep_bod_disable() {
    MCUCR_t tmp;
    tmp |= (BODS | BODSE);
    MCUCR = tmp;
    tmp &= ~BODSE;
    MCUCR = tmp;
}

void HAL::Atmel::Impl::sleep(SleepMode mode) {
    auto adcsraSave = ADCSRA;
    ADEN.clear(); // disable the ADC
    switch(mode) {
    case SleepMode::POWER_DOWN:
        SMCR.apply(~SM0 | SM1 | ~SM2); break;
    case SleepMode::STANDBY:
        SMCR.apply(~SM0 | SM1 | SM2); break;
    case SleepMode::IDLE:
        SMCR.apply(~SM0 | ~SM1 | ~SM2); break;
    }

    {
        AtomicScope::SEI _;
        SE.set(); // sleep enable
        sleep_bod_disable();
    }
    sleep_cpu();
    SE.clear(); // sleep disable
    ADCSRA = adcsraSave;
}
