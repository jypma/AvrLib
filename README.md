BUGS
====


TODO
====

- try out a mkISR(obj,pbj,opbj... macro that auto-generates the ISR's, without the need for pointers

- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
