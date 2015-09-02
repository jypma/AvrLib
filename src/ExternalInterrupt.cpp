#include "ExternalInterrupt.hpp"

InterruptChain extInt0;
InterruptChain extInt1;

uint8_t int1_invocations = 0;

ISR(INT0_vect) {
  extInt0.invoke();
}

ISR(INT1_vect) {
    int1_invocations++;
  extInt1.invoke();
}
