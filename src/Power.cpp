#include "HAL/Atmel/Power.hpp"

using namespace HAL::Atmel;

void Power::watchdogInterrupts (int8_t mode) {
    // correct for the fact that WDP3 is *not* in bit position 3!
    if (mode & _BV(3))
        mode ^= _BV(3) | _BV(WDP3);
    // pre-calculate the WDTCSR value, can't do it inside the timed sequence
    // we only generate interrupts, no reset
    uint8_t wdtcsr = mode >= 0 ? _BV(WDIE) | mode : 0;
    MCUSR &= ~(1<<WDRF);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
#ifndef WDTCSR
#define WDTCSR WDTCR
#endif
        WDTCSR |= (1<<WDCE) | (1<<WDE); // timed sequence
        WDTCSR = wdtcsr;
    }
}

void Power::powerDown () {
    uint8_t adcsraSave = ADCSRA;
    ADCSRA &= ~ _BV(ADEN); // disable the ADC
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        sleep_enable();
        sleep_bod_disable(); // can't use this - not in my avr-libc version!
//#ifdef BODSE
//        MCUCR = MCUCR | _BV(BODSE) | _BV(BODS); // timed sequence
//        MCUCR = (MCUCR & ~ _BV(BODSE)) | _BV(BODS);
#//endif
    }
    sleep_cpu();
    sleep_disable();
    // re-enable what we disabled
    ADCSRA = adcsraSave;
}
