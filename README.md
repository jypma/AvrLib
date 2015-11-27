BUGS
====
 - find out why pulseCounter.minimumLength is somehow applied x2

TODO
====
 - Replace << and >> streaming operators with template methods, so we can get rid of *Invocations on Reader and Writer
     in().expect<>()            // without target instance    DONE
     in().readAs<Format<>>(t);  // with target instance       DONE
     scan()                     // multiple options           DONE
     out().write<>()            // without target instance    TODO
     out().writeAs<Format<>>(t) // with target instance       TODO
   Create types for the missing operator variants.
     
 - Allow Format<Format<X,Y>> to be equivalent to Format<X,Y>, so we can declare all format-dependent methods
   to take varargs directly, where appropriate. 
 - Rewrite SerialConfig to be a static template class, and remove (for now) ability to change serial configs at
   runtime. That'll create much faster software serial, and removes the need to juggle pointers in the fifo.
 - UsartFifo: only enable the bit once after the first write in a writer, rather than for every byte
 - investigate replacing INTERRUPT_HANDLER1, 2, etc. macros with a macro-block
     INTERRUPT_HANDLERS {   // starts a nested type with a single, known name
       INTERRUPT_HANDLER(vect, method);  // declares a single method with a known name, vect is template param for the method.. but what about multi?
     };
 - use the above for an easy interrupt forwarding macro
  
 - maintain "last sent" pin value in pulse counter, comparing with pin->isHigh() after each change, inserting dummies on mismatch
   . That way we don't need the double memory anymore after all.
 - consider pin factory methods to imply default state as input / output
 - make namespace HAL::Atmel consistent (everything public goes there, everything private in sub-namespaces)

- Change fifo.abortedWrites into a flag "overflow", and add two other flags "full" and "hasData", so
  fastread() and fastwrite() don't have to invoke complex calculations.
- Split up AbstractFifo API into a reading and writing part, that both take a pointer to the actual data.
  That way, Fifo delegates like the UartTxPin can simply extend those read/write base classes.
- create template variants of the PinChange handlers that don't allow runtime setting of onRising / onFalling
  but rather do it in template, to save cycles.
  