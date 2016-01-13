BUGS
====
 - find out why pulseCounter.minimumLength is somehow applied x2

TODO
====
 - Allow Format<Format<X,Y>> to be equivalent to Format<X,Y>, so we can declare all format-dependent methods
   to take varargs directly, where appropriate. 
 - Allow STR("") where Token<STR("")> is expected
 - Replace << and >> streaming operators with template methods, so we can get rid of *Invocations on Reader and Writer
     in().expect<>()            // without target instance    DONE
     in().readAs<Format<>>(t);  // with target instance       DONE
     scan()                     // multiple options           DONE
     out().write<>()            // without target instance    TODO
     out().writeAs<Format<>>(t) // with target instance       TODO
     
   Entries inside a Format<...> WITHOUT target instance:
     literal or primitive variable            (write only)
     struct that has a ::Protocol             (write only)
     &primitive variable                      (read only)
     &struct that has a ::Protocol            (read only)
     STR("abc"), implying Token<STR("abc")>   (read+write)
     PADDING(4), ignored bytes when reading, zero when writing
     
   Entries inside a Format<...> WITH a target instance:

--------------------------------------------------------------------------------

General
-------
   dec() now takes either a primitive& (resulting in Decimal) or a const primitive& (resulting in ConstDecimal)

Read ad-hoc
-----------

Literal (token), STR("abc") or F("abc"), &primitive variable, &struct that has a ::Protocol, Padding (skip bytes), as<MyFormat>(&struct), dec(&primitive variable)

    TypeWithFormat instance;
    uint16_t value;
    ReaderResult result = fifo.read(uint8_t(15), F("abc"), instance, padding(2), dec(value));

Write ad-hoc
------------

Literal (token), STR("abc") or F("abc"), primitive variable, const &struct that has a ::Protocol, Padding (skip bytes), as<MyFormat>(const &struct)

    const TypeWithFormat instance;
    const uint16_t value;
    bool result = fifo.write(uint8_t(15), F("abc"), instance, padding(2), dec(value));

Format
------

Literal (token), STR("abc") or F("abc"), pointer-to-primitive-field, pointer-to-struct that has a ::Protocol, Padding (skip bytes), as<MyFormat>(&struct)

    typedef Protocol<This> P;

    typedef P<
      P::ByteString<8>,
      STR("abc"),
      P::Binary<uint8_t, This::*a>,
      P::Padding<2>
    > Format;

Scan from fifo
--------------

	scan(fifo, [] (auto s) {
	    uint16_t value;
	    onMatch(s, uint8_t(15), F("abc"), &value, padding(2), [] {
	       // ...
	    });
	});


Scan from chunked fifo
----------------------

	scan(chunkedFifo, [] (auto s) {
	    uint16_t value;
	    onMatch(s, uint8_t(15), F("abc"), &value, padding(2), [] {
	       // ...
	    });    
    });


--------------------------------------------------------------------------------
   
   
-  Add JSON (or protobuf?) types for easier packet output.
     
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
  