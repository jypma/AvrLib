[![Build Status](https://travis-ci.org/jypma/AvrLib.svg?branch=master)](https://travis-ci.org/jypma/AvrLib)

AvrLib
======

This is a C++14 library for embedded development on low-memory 8-bit AVR microcontrollers. It currently focuses on ATMega328, but other hardware definitions can be added fairly easily. It attempts to offer the highest amount of compile-time safety possible with modern C++, without sacrificing performance. This generally results in nicely readable code, and inherint unit testability. 

Specifically, we follow the following patterns:

- Use templates for dependency injection - This allows the logic of most non-hardware code to be fully unit tested, while 
  pusing down real hardware stuff to a HAL layer
- Compile-time time constants like `10_sec`, which will hit static assertions if they cause overflows or underflows when
  used on hardware timers with known prescalers.
- No heap. All allocations are statically known on compile time, and FIFOs are used to communicate e.g. radio packets.
- Arduino and JeeLib compatible pin numbers, but compile-time safe capabilities. E.g. it's only possible to invoke PWM settings on pins that actually have hardware PWM, and only if a timer has been set up to do so.

TODO
====
 - reading DS18B20 hangs if debug logging is blocked on a full output FIFO..... and shouldn't.
 - Do what [yalla](https://github.com/chrism333/yalla/blob/master/include/yalla/device/atmega8/avr/io.hpp) does,
   in order to remain compilable on avr-gcc 6+, which no longer accepts avr-libc's "reinterpret_cast_ inside
   constexpr (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63715).
 - make a timeout on the RFM12 failing to initialize (INT pin not going high)
 - re-investigate if we can have write(myStruct) instead of write(&myStruct) when decltype(struct)::Protocol exists 
 - Have ChunkedFifo maintain its own Fifo of fixed size internally, to workaround the -Os "bug"
   (or maybe data is a temp that's copied and gone??)
 - log::debug should always invoke toString(T), which defaults to dec() for decimals,
   but also defines Microsecond<> as "...ms", etc.
 - Make RFM12 OOK support optional, so you can create the RFM12 driver without 
   using a comparator with suitable timer prescaler
 - Scan for chunked-fifo like API, drop whole chunk if not matched. 
      Or, if scan can work within a chunk, make up a different name, e.g. match()
      
 - Create a TypedFifo, templated on sizeof(T), without the generic write and read methods; 
    * only one T at a time
    * maybe on(T, lambda)
 - find out why pulseCounter.minimumLength is somehow applied x2     
 - Rewrite SerialConfig to be a static template class, and remove (for now) ability to change serial configs at
   runtime. That'll create much faster software serial, and removes the need to juggle pointers in the fifo.
  
 - maintain "last sent" pin value in pulse counter, comparing with pin->isHigh() after each change, inserting dummies on mismatch
   . That way we don't need the double memory anymore after all.
 - consider pin factory methods to imply default state as input / output
 - make namespace HAL::Atmel consistent (everything public goes there, everything private in sub-namespaces)

- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- create template variants of the PinChange handlers that don't allow runtime setting of onRising / onFalling
  but rather do it in template, to save cycles.
