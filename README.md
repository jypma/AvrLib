BUGS
====


TODO
====
 - reimplement pin change interrupts
      * define a custom "interrupt" to bind the virtual pin-change interrupt, to serve as typename PinA0::INT
      * make PinA0 etc. optionally each implement PCINT2 interrupt
           when change is detected, invoke _Table::customINT_FOR_PinA0 etc.
      
 - consider pin factory methods to imply default state as input / output
 - make namespace HAL::Atmel consistent (everything public goes there, everything private in sub-namespaces)
 - Implement rate limiting for PulseCounter interrupts
- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
