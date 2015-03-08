#include "ExternalInterrupt.hpp"

InterruptChain extInt0;
InterruptChain extInt1;

ISR(INT0_vect) {
  extInt0.invoke();
}

ISR(INT1_vect) {
  extInt1.invoke();
}
