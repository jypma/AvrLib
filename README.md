BUGS
====


TODO
====
- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- Simplify PinPD0<Usart0> pinPD0(usart0) : just have two constructors, one with usart0, one without? or factory method?
- Convert Usart constructor arg to be template, and add compile-time checks
- try out a mkISR(obj,pbj,opbj... macro that auto-generates the ISR's, without the need for pointers

- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
