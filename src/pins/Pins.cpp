#include "pins/Pins.hpp"

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

volatile void _configurePin0AsGPIO() {
    // Turn off transmitter, to allow pin 0 to be used as GPIO
    UCSR0B &= ~(1 << TXEN0);
}

volatile void _configurePin1AsGPIO() {
    // Turn off receiver, to allow pin 1 to be used as GPIO
    UCSR0B &= ~(1 << RXEN0);
}

volatile void _noop() {}
