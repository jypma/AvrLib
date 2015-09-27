BUGS
====


TODO
====
   consider pin factory methods to imply default state as input / output
 - make namespace HAL::Atmel consistent (everything public goes there, everything private in sub-namespaces)
 - Implement rate limiting for PulseCounter interrupts
 - create constexpr constructor DSL for timers, e.g. auto timer0 = Timer0().withPrescaler<8>.inNormalMode() 
- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- Simplify PinPD0<Usart0> pinPD0(usart0) : just have two constructors, one with usart0, one without? or factory method?
- Convert Usart constructor arg to be template, and add compile-time checks

- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
