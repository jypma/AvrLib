BUGS
====
 - find out why pulseCounter.minimumLength is somehow applied x2

TODO
====
 - consider pin factory methods to imply default state as input / output
 - make namespace HAL::Atmel consistent (everything public goes there, everything private in sub-namespaces)

- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
- create template variants of the PinChange handlers that don't allow runtime setting of onRising / onFalling
  but rather do it in template, to save cycles.
  