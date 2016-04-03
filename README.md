BUGS
====
 - Decimal format for time: rt.millis() etc. for debug output
 - Create a TypedFifo, templated on sizeof(T), without the generic write and read methods; 
    * only one T at a time
    * maybe on(T, lambda)
 - refactor out MockComparator and MockRealTimer
 - find out why pulseCounter.minimumLength is somehow applied x2
 - Have F("12") for length <= 2 (or 3) just be plain C strings, since they use as much mem but much faster.   
-  Add JSON (or protobuf?) types for easier packet output.
     
 - Rewrite SerialConfig to be a static template class, and remove (for now) ability to change serial configs at
   runtime. That'll create much faster software serial, and removes the need to juggle pointers in the fifo.
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
- create template variants of the PinChange handlers that don't allow runtime setting of onRising / onFalling
  but rather do it in template, to save cycles.
  