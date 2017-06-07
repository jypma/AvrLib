[![Build Status](https://travis-ci.org/jypma/AvrLib.svg?branch=master)](https://travis-ci.org/jypma/AvrLib)

AvrLib
======

This is a C++14 library for embedded development on low-memory 8-bit AVR microcontrollers. It currently focuses on ATMega328, but other hardware definitions can be added fairly easily. It attempts to offer the highest amount of compile-time safety possible with modern C++, without sacrificing performance. This generally results in nicely readable code, and inherint unit testability. 

Specifically, we follow the following patterns:

- Use templates for dependency injection - This allows the logic of most non-hardware code to be fully unit tested, while 
  pusing down real hardware stuff to a HAL layer
- Compile-time time constants like `10_sec`, which will hit static assertions if they cause overflows or underflows when
  used on hardware timers with known prescalers.
- Inlineable interrupt handlers that can be normal class methods.
- No heap. All allocations are statically known on compile time, and FIFOs are used to communicate e.g. radio packets.
- Arduino and JeeLib compatible pin numbers, but compile-time safe capabilities. E.g. it's only possible to invoke PWM settings on pins that actually have hardware PWM, and only if a timer has been set up to do so.

Getting started
===============

In order to write a program against AvrLib, you'll want to pull it as a sub-module into your own git repository. That will ensure you working against a stable version, while also having full source-code access.

Usage
=====

We'll explain a few of the patterns used in this library in further detail here

Templates for dependency injection
----------------------------------

Say that you're encapsulating some behaviour into a class, which needs to toggle a pin. Since on AVR and other microcontrollers, different pins have different features, we like to express that with actual types. However, we don't want the runtime overhead of dynamic dispatch, so `virtual` methods are out of the question. Now, we can achieve something similar by declaring our actual class a template, and having the pin's concrete type be a type argument:

```C++
class PinPD2 {
  inline bool isHigh() { ... }
};

template <typename pin_t>
class Button {
  pin_t * const pin;
  
  bool isPressed() { return pin->isLow(); }
};

PinPD2 pinPD2;
Button<PinPD2> plusButton = { pinPD2 };
```

This way, our `Button` class is decoupled from its actual pin. Yet, when we instantiate
the button, the compiler can drop in the methods, and even inline them if so desired.

Also, we can write a mock implementation of the pin class, which we can use in unit tests.

Compile-time prescalers and time constants
------------------------------------------

The compiler knows about the clock speed of the microcontroller, and about the prescaler
used for timers. Combined with C++ user-defined literals, we can now make fairly smart constants:

```C++
Timer0::withPrescaler<1024>::inNormalMode timer0 = {};
RealTimer<decltype(timer0)> rt = { timer0 };
VariableDeadline<decltype(rt)> deadline = { rt };

deadline.schedule(1_ms);     // OK, becomes a static number of timer ticks during compilation
deadline.schedule(1_us);     // Compile error -> delay is too short for this timer
deadline.schedule(1000_min); // Compile error -> delay is too long for this timer
```

There are `static_assert` messages in place that will inform the user to lower or raise
the timer prescaler, when trying to use it for delays that round to 0 or 1 timer tick,
or would overflow the target integer used (typically `uint16_t` or `uint32_t`).

Inlineable interrupt handlers on class types
--------------------------------------------

Interrupt handlers are usually fairly tightly coupled to only a specific subsystem of our embedded code. For example, the pin change handler for a button class should only affect the button, and nothing else. Yet, traditionally, interrupt handlers need to be globally defined, which is error-prone and not unit-testable.

Using dynamic dispatch for interrupt handlers is not an alternative in many cases, as it prevents inlining, and hence (on AVR) makes the interrupt handlers much slower, since _ALL_ registers must be put onto the stack, rather than only the few ones actually changed.

AvrLib tackles this problem by a combination of a few macros and recursive templates. Let's take our button class as an example. It can define an interrupt handler for its pin like this:

```C++
template <typename pin_t>
class Button {
  void onPinChange { ... }
public:
  typedef On<Button, typename pin_t::INT, &This::onPinChange> Handlers;
};
```

The convention is based on the following:
- The framework provides an `On<type, interrupt_type, method>` template to bind an interrupt vector to an instance method.
- The instantiation of that template must be called `Handlers`.
- Actual pin types provide a `INT` nested type that identifies which interrupt vector is the pin change handler for that pin. Other interrupt vector are similarly bound to actual C++ types.

The root application is also modeled as a class:

```C++
class App {
  PinPD2 pinPD2;
  Button<PinPD2> plusButton = { pinPD2 };
public:
  typedef Delegate<App, decltype(plusButton), &App::plusButton> Handlers;
  
  void main() { /* main loop goes here */ }
};
```

Here, a second template `Delegate<type, field_type, field>` is used to declare delegating interrupt vectors to actual handlers on a field.

Finally, a root `main.cpp` will declare a small macro to actually instantiate and bind the main `App` type:

```C++
RUN_APP(App)
```

The above will instantiate an `App` and run its main loop, but also discover the `Handlers` type it has declared, and auto-generate all gcc interrupt vectors to call into their respective implementations. Since all of that is statically known at compile time, _most_ interrupt handlers can actually be fully in-lined that way.

Examples
========

The [AvrLibDemo](https://github.com/jypma/AvrLibDemo/) project gathers a lot of example code for actual hardware projects that use this library. It also comes with a generic [Makefile](https://github.com/jypma/AvrLibDemo/blob/master/tools/common.mk) that's shared between all projects.


