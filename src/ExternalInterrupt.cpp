#include "ExternalInterrupt.hpp"

InterruptHandler extInt0;
InterruptHandler extInt1;

ISR(INT0_vect) {
  extInt0.invoke();
}

ISR(INT1_vect) {
  extInt1.invoke();
}
