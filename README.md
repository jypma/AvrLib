BUGS
====


TODO
====

- move microseconds2ticks out of prescaler into the Microsecond classes

- Add timeouts to ESP8266
- Make PulseCounter factory method nicer, both maximum fifo size and mininumLength should be conveniently passable.
- Time handling, so comparisons can be written nicer than "if (pulse.getDuration() > (100_us).template toCounts<typeof tm1>()) {".
- Especially get rid of .template toCounts().
- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
- Split up periodic() into withFixedDelay() and periodic()
- Add initialDelay to periodic()
