#include "Pins.hpp"

Callback int_0_callback;
Callback int_1_callback;

SIGNAL(INT0_vect) {
  if (int_0_callback) {
      int_0_callback();
  }
}

SIGNAL(INT1_vect) {
  if(int_1_callback) {
      int_1_callback();
  }
}

