BUGS
====


TODO
====

reverse arguments and rename toCountsOn :    toCounts(const duration_t, const prescaled_t &)
- try out a mkISR(obj,pbj,opbj... macro that auto-generates the ISR's, without the need for pointers

- Add timeouts to ESP8266
- Time handling, so comparisons can be written nicer than "if (pulse.getDuration() > (100_us).template toCounts<typeof tm1>()) {".
- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
