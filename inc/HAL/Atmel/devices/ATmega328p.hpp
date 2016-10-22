#ifndef HAL_ATMEL_DEVICES_ATMEGA328P_HPP_
#define HAL_ATMEL_DEVICES_ATMEGA328P_HPP_

#include <HAL/Atmel/Registers.hpp>
#include "Time/Prescaled.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "HAL/Atmel/Usart.hpp"
#include "HAL/Atmel/Pin.hpp"
#include "HAL/Atmel/TWI.hpp"
#include "HAL/Atmel/SleepMode.hpp"

namespace HAL {
namespace Atmel {
namespace Info {

using namespace HAL::Atmel::Registers;

struct Power {
	void set_sleep_mode(SleepMode mode) {
        switch(mode) {
        case SleepMode::POWER_DOWN:
            SMCR.apply(~SM2 | SM1 | ~SM0); break;
        case SleepMode::STANDBY:
            SMCR.apply(SM2 | SM1 | ~SM0); break;
        case SleepMode::IDLE:
            SMCR.apply(~SM2 | ~SM1 | ~SM0); break;
        }
	}
};

struct Int0Info {
    typedef Int_INT0_ INT;

    static constexpr auto INT_r = INT0;
    static constexpr auto INTF = INTF0;
    static constexpr auto ISC0 = ISC00;
    static constexpr auto ISC1 = ISC01;
    static constexpr auto EICR = EICRA;
};

struct Int1Info {
    typedef Int_INT1_ INT;

    static constexpr auto INT_r = INT1;
    static constexpr auto INTF = INTF1;
    static constexpr auto ISC0 = ISC10;
    static constexpr auto ISC1 = ISC11;
    static constexpr auto EICR = EICRA;
};

struct PCInt0Info {
    typedef Int_PCINT0_ PCINT;
    static constexpr auto PCIE = PCIE0;
    static constexpr auto PIN = PINB;
    static constexpr auto PCMSK = PCMSK0;
};

struct PCInt1Info {
    typedef Int_PCINT1_ PCINT;
    static constexpr auto PCIE = PCIE1;
    static constexpr auto PIN = PINC;
    static constexpr auto PCMSK = PCMSK1;
};

struct PCInt2Info {
    typedef Int_PCINT2_ PCINT;
    static constexpr auto PCIE = PCIE2;
    static constexpr auto PIN = PIND;
    static constexpr auto PCMSK = PCMSK2;
};

struct Usart0Info {
	static constexpr auto U2X = U2X0;
	static constexpr auto UBRR = UBRR0;
	static constexpr auto TXEN = TXEN0;
	static constexpr auto RXEN = RXEN0;
	static constexpr auto RXCIE = RXCIE0;
	static constexpr auto UDRIE = UDRIE0;
	static constexpr auto UCSRC = UCSR0C;
	static constexpr auto UCSZ0 = UCSZ00;
	static constexpr auto UCSZ1 = UCSZ01;
	static constexpr auto TXC = TXC0;
	static constexpr auto UDR = UDR0;
};

} // namespace Info

enum class ExtPrescaler: uint8_t { _1, _8, _64, _256, _1024 };

template <uint16_t i> struct ExtPrescalerFromInt {};
template<> struct ExtPrescalerFromInt<1> { static constexpr ExtPrescaler value = ExtPrescaler::_1; };
template<> struct ExtPrescalerFromInt<8> { static constexpr ExtPrescaler value = ExtPrescaler::_8; };
template<> struct ExtPrescalerFromInt<64> { static constexpr ExtPrescaler value = ExtPrescaler::_64; };
template<> struct ExtPrescalerFromInt<256> { static constexpr ExtPrescaler value = ExtPrescaler::_256; };
template<> struct ExtPrescalerFromInt<1024> { static constexpr ExtPrescaler value = ExtPrescaler::_1024; };

enum class IntPrescaler: uint8_t { _1, _8, _32, _64, _128, _256, _1024 };

template <uint16_t i> struct IntPrescalerFromInt {};
template<> struct IntPrescalerFromInt<1> { static constexpr IntPrescaler value = IntPrescaler::_1; };
template<> struct IntPrescalerFromInt<8> { static constexpr IntPrescaler value = IntPrescaler::_8; };
template<> struct IntPrescalerFromInt<32> { static constexpr IntPrescaler value = IntPrescaler::_32; };
template<> struct IntPrescalerFromInt<64> { static constexpr IntPrescaler value = IntPrescaler::_64; };
template<> struct IntPrescalerFromInt<128> { static constexpr IntPrescaler value = IntPrescaler::_128; };
template<> struct IntPrescalerFromInt<256> { static constexpr IntPrescaler value = IntPrescaler::_256; };
template<> struct IntPrescalerFromInt<1024> { static constexpr IntPrescaler value = IntPrescaler::_1024; };

} // namespace Atmel
} // namespace HAL

namespace Time {

template<> struct PrescalerMeta<HAL::Atmel::ExtPrescaler,HAL::Atmel::ExtPrescaler::_1> {
    constexpr static uint8_t power2 = 0;
};

template<> struct PrescalerMeta<HAL::Atmel::ExtPrescaler,HAL::Atmel::ExtPrescaler::_8> {
    constexpr static uint8_t power2 = 3;
};

template<> struct PrescalerMeta<HAL::Atmel::ExtPrescaler,HAL::Atmel::ExtPrescaler::_64> {
    constexpr static uint8_t power2 = 6;
};

template<> struct PrescalerMeta<HAL::Atmel::ExtPrescaler,HAL::Atmel::ExtPrescaler::_256> {
    constexpr static uint8_t power2 = 8;
};

template<> struct PrescalerMeta<HAL::Atmel::ExtPrescaler,HAL::Atmel::ExtPrescaler::_1024> {
    constexpr static uint8_t power2 = 10;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_1> {
    constexpr static uint8_t power2 = 0;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_8> {
    constexpr static uint8_t power2 = 3;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_32> {
    constexpr static uint8_t power2 = 5;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_64> {
    constexpr static uint8_t power2 = 6;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_128> {
    constexpr static uint8_t power2 = 7;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_256> {
    constexpr static uint8_t power2 = 8;
};

template<> struct PrescalerMeta<HAL::Atmel::IntPrescaler,HAL::Atmel::IntPrescaler::_1024> {
    constexpr static uint8_t power2 = 10;
};

} namespace HAL { namespace Atmel { namespace Info {

struct Timer0Info {
    typedef Int_TIMER0_OVF_ INT;

    static constexpr auto TCNT = TCNT0;
    static constexpr auto TOV = TOV0;
    static constexpr auto TOIE = TOIE0;

    typedef uint8_t value_t;
    typedef ExtPrescaler prescaler_t;
    template <uint16_t i> using PrescalerFromInt = ExtPrescalerFromInt<i>;

    static void setPrescaler(prescaler_t p) {
    	switch(p) {
    	case ExtPrescaler::_1    : TCCR0B.apply( CS00 | ~CS01 | ~CS02); break;
    	case ExtPrescaler::_8    : TCCR0B.apply(~CS00 |  CS01 | ~CS02); break;
    	case ExtPrescaler::_64   : TCCR0B.apply( CS00 |  CS01 | ~CS02); break;
    	case ExtPrescaler::_256  : TCCR0B.apply(~CS00 | ~CS01 |  CS02); break;
    	case ExtPrescaler::_1024 : TCCR0B.apply( CS00 | ~CS01 |  CS02); break;
    	}
    }

    static void configureNormal(prescaler_t p) {
        WGM00.clear(); // Mode 0, normal
        WGM01.clear();
        setPrescaler(p);
    }

    static void configureFastPWM(prescaler_t p) {
        WGM00.set(); // Mode 3, count up to 0xFF
        WGM01.set();
        setPrescaler(p);
    }

    struct Comparator {
        typedef Timer0Info timer_info_t;
        typedef timer_info_t::value_t value_t;
    };

    struct ComparatorA: public Comparator {
        typedef Int_TIMER0_COMPA_ INT;
        static constexpr auto OCR = OCR0A;
        static constexpr auto OCIE = OCIE0A;
        static constexpr auto OCF = OCF0A;
        static constexpr auto COM0 = COM0A0;
        static constexpr auto COM1 = COM0A1;
        static constexpr auto FOC = FOC0A;
    };
    struct ComparatorB: public Comparator {
        typedef Int_TIMER0_COMPB_ INT;
        static constexpr auto OCR = OCR0B;
        static constexpr auto OCIE = OCIE0B;
        static constexpr auto OCF = OCF0B;
        static constexpr auto COM0 = COM0B0;
        static constexpr auto COM1 = COM0B1;
        static constexpr auto FOC = FOC0B;
    };
};

struct Timer1Info {
    typedef Int_TIMER1_OVF_ INT;

    static constexpr auto TCNT = TCNT1;
    static constexpr auto TOV = TOV1;
    static constexpr auto TOIE = TOIE1;

    typedef uint16_t value_t;
    typedef ExtPrescaler prescaler_t;
    template <uint16_t i> using PrescalerFromInt = ExtPrescalerFromInt<i>;

    static void setPrescaler(prescaler_t p) {
    	switch(p) {
    	case ExtPrescaler::_1    : TCCR1B.apply( CS10 | ~CS11 | ~CS12); break;
    	case ExtPrescaler::_8    : TCCR1B.apply(~CS10 |  CS11 | ~CS12); break;
    	case ExtPrescaler::_64   : TCCR1B.apply( CS10 |  CS11 | ~CS12); break;
    	case ExtPrescaler::_256  : TCCR1B.apply(~CS10 | ~CS11 |  CS12); break;
    	case ExtPrescaler::_1024 : TCCR1B.apply( CS10 | ~CS11 |  CS12); break;
    	}
    }

    static void configureNormal(prescaler_t p) {
        WGM10.clear();
        WGM11.clear();
        WGM12.clear();
        WGM13.clear();
        setPrescaler(p);
    }

    inline static void configureFastPWM(prescaler_t p) {
        WGM10.clear();
        WGM11.set();
        WGM12.set();
        WGM13.set();
        setPrescaler(p);
        ICR1.set(0xFFFF); // Mode 14, count up to ICR1, which we always set at 0xFFFF.
    }

    struct Comparator {
        typedef Timer1Info timer_info_t;
        typedef timer_info_t::value_t value_t;
    };

    struct ComparatorA: public Comparator {
        typedef Int_TIMER1_COMPA_ INT;
        static constexpr auto OCR = OCR1A;
        static constexpr auto OCIE = OCIE1A;
        static constexpr auto OCF = OCF1A;
        static constexpr auto COM0 = COM1A0;
        static constexpr auto COM1 = COM1A1;
        static constexpr auto FOC = FOC1A;
    };
    struct ComparatorB: public Comparator {
        typedef Int_TIMER1_COMPB_ INT;
        static constexpr auto OCR = OCR1B;
        static constexpr auto OCIE = OCIE1B;
        static constexpr auto OCF = OCF1B;
        static constexpr auto COM0 = COM1B0;
        static constexpr auto COM1 = COM1B1;
        static constexpr auto FOC = FOC1B;
    };
};

struct Timer1LoInfo: public Timer1Info {
    static constexpr auto TCNT = TCNT1L;
    typedef uint8_t value_t;

    struct Comparator {
        typedef uint16_t value_t;
        typedef Timer1Info timer_info_t;
    };

    struct ComparatorA: public Timer1Info::ComparatorA {
        typedef Timer1LoInfo timer_info_t;
        static constexpr auto OCR = OCR1AL;
    };
    struct ComparatorB: public Timer1Info::ComparatorB {
        typedef Timer1LoInfo timer_info_t;
        static constexpr auto OCR = OCR1BL;
    };
};

struct Timer2Info {
    typedef Int_TIMER2_OVF_ INT;

    static constexpr auto TCNT = TCNT2;
    static constexpr auto TOV = TOV2;
    static constexpr auto TOIE = TOIE2;

    typedef uint8_t value_t;
    typedef IntPrescaler prescaler_t;
    template <uint16_t i> using PrescalerFromInt = IntPrescalerFromInt<i>;

    static void setPrescaler(prescaler_t p) {
    	switch(p) {
    	case IntPrescaler::_1    : TCCR2B.apply( CS20 | ~CS21 | ~CS22); break;
    	case IntPrescaler::_8    : TCCR2B.apply(~CS20 |  CS21 | ~CS22); break;
    	case IntPrescaler::_32   : TCCR2B.apply( CS20 |  CS21 | ~CS22); break;
    	case IntPrescaler::_64   : TCCR2B.apply(~CS20 | ~CS21 |  CS22); break;
    	case IntPrescaler::_128  : TCCR2B.apply( CS20 | ~CS21 |  CS22); break;
    	case IntPrescaler::_256  : TCCR2B.apply(~CS20 |  CS21 |  CS22); break;
    	case IntPrescaler::_1024 : TCCR2B.apply( CS20 |  CS21 |  CS22); break;
    	}
    }

    static void configureNormal(prescaler_t p) {
        WGM20.clear();
        WGM21.clear();
        setPrescaler(p);
    }

    static void configureFastPWM(prescaler_t p) {
        WGM20.set();
        WGM21.set();
        setPrescaler(p);
    }

    struct Comparator {
        typedef Timer2Info timer_info_t;
        typedef timer_info_t::value_t value_t;
    };

    struct ComparatorA: public Comparator {
        typedef Int_TIMER2_COMPA_ INT;
        static constexpr auto OCR = OCR2A;
        static constexpr auto OCIE = OCIE2A;
        static constexpr auto OCF = OCF2A;
        static constexpr auto COM0 = COM2A0;
        static constexpr auto COM1 = COM2A1;
        static constexpr auto FOC = FOC2A;
    };
    struct ComparatorB: public Comparator {
        typedef Int_TIMER2_COMPB_ INT;
        static constexpr auto OCR = OCR2B;
        static constexpr auto OCIE = OCIE2B;
        static constexpr auto OCF = OCF2B;
        static constexpr auto COM0 = COM2B0;
        static constexpr auto COM1 = COM2B1;
        static constexpr auto FOC = FOC2B;
    };
};

template <uint8_t bit>
struct PinOnPortD {
    static constexpr auto DDR = DDRD.bit<bit>();
    static constexpr auto PORT = PORTD.bit<bit>();
    static constexpr auto PIN = PIND.bit<bit>();
    typedef Info::PCInt2Info pcintInfo;
    static constexpr uint8_t pcintBit = bit;
};

template <uint8_t bit>
struct PinOnPortB {
    static constexpr auto DDR = DDRB.bit<bit>();
    static constexpr auto PORT = PORTB.bit<bit>();
    static constexpr auto PIN = PINB.bit<bit>();
    typedef Info::PCInt0Info pcintInfo;
    static constexpr uint8_t pcintBit = bit;
};

template <uint8_t bit>
struct PinOnPortC {
    static constexpr auto DDR = DDRC.bit<bit>();
    static constexpr auto PORT = PORTC.bit<bit>();
    static constexpr auto PIN = PINC.bit<bit>();
    typedef Info::PCInt1Info pcintInfo;
    static constexpr uint8_t pcintBit = bit;
};

struct PinPD0Info: public PinOnPortD<0> {
    typedef Usart0Info usart_info_t;

    static inline void configureAsGPIO() {
    	RXEN0.clear();   // disable hardware USART receiver
    }
};

struct PinPD1Info: public PinOnPortD<1> {
    typedef Usart0Info usart_info_t;

    static inline void configureAsGPIO() {
    	TXEN0.clear();  // disable hardware USART transmitter
    }
};

struct GPIOPin {
    static inline void configureAsGPIO() {}
};

struct PinOnTimer0 {
    typedef Timer0Info timer_info_t;
};

struct PinOnTimer1 {
    typedef Timer1Info timer_info_t;
};

struct PinOnTimer2 {
    typedef Timer2Info timer_info_t;
};


struct PinPD2Info: public PinOnPortD<2>, public GPIOPin {};
struct PinPD3Info: public PinOnPortD<3>, public GPIOPin, public PinOnTimer2 {};
struct PinPD4Info: public PinOnPortD<4>, public GPIOPin {};
struct PinPD5Info: public PinOnPortD<5>, public GPIOPin, public PinOnTimer0 {};
struct PinPD6Info: public PinOnPortD<6>, public GPIOPin, public PinOnTimer0 {};
struct PinPD7Info: public PinOnPortD<7>, public GPIOPin {};
struct PinPB0Info: public PinOnPortB<0>, public GPIOPin {};
struct PinPB1Info: public PinOnPortB<1>, public GPIOPin, public PinOnTimer1 {};
struct PinPB2Info: public PinOnPortB<2>, public GPIOPin, public PinOnTimer1 {};
struct PinPB3Info: public PinOnPortB<3>, public GPIOPin, public PinOnTimer2 {};
struct PinPB4Info: public PinOnPortB<4>, public GPIOPin {};
struct PinPB5Info: public PinOnPortB<5>, public GPIOPin {};

struct PinPC0Info: public PinOnPortC<0>, public GPIOPin {
  static constexpr auto adc_mux = ~(MUX0 | MUX1 | MUX2 | MUX3);
};
struct PinPC1Info: public PinOnPortC<1>, public GPIOPin {
  static constexpr auto adc_mux = MUX0 | ~MUX1 | ~MUX2 | ~MUX3;
};
struct PinPC2Info: public PinOnPortC<2>, public GPIOPin {
  static constexpr auto adc_mux = ~MUX0 | MUX1 | ~MUX2 | ~MUX3;
};
struct PinPC3Info: public PinOnPortC<3>, public GPIOPin {
  static constexpr auto adc_mux = MUX0 | MUX1 | ~MUX2 | ~MUX3;
};
struct PinPC4Info: public PinOnPortC<4>, public GPIOPin {
  static constexpr auto adc_mux = ~MUX0 | ~MUX1 | MUX2 | ~MUX3;
};
struct PinPC5Info: public PinOnPortC<5>, public GPIOPin {
  static constexpr auto adc_mux = MUX0 | ~MUX1 | MUX2 | ~MUX3;
};
struct PinA6Info {
  static constexpr auto adc_mux = ~MUX0 | MUX1 | MUX2 | ~MUX3;
};
struct PinA7Info {
  static constexpr auto adc_mux = MUX0 | MUX1 | MUX2 | ~MUX3;
};

} // namespace Info

////////////////////////////// TIMERS ///////////////////////////////////////////////////////

typedef Timer::TimerDeclaration<Info::Timer0Info,ExtPrescaler::_1> Timer0;
struct Timer1: public Timer::TimerDeclaration<Info::Timer1Info,ExtPrescaler::_1> {
	typedef Timer::TimerDeclaration<Info::Timer1LoInfo,ExtPrescaler::_1> lowByte;
};
typedef Timer::TimerDeclaration<Info::Timer2Info,IntPrescaler::_1> Timer2;

////////////////////////////// PINS /////////////////////////////////////////////////////////
/**
 * Declares Usart0 to be used.
 * In order to actually send or receive data, declare types and instances of PinPD1 or PinPD0.
 * Pass the baud rate as constructor argument, e.g.
 *
 *     Usart0 usart0(57600)
 */
typedef Usart<Info::Usart0Info> Usart0;

template <typename usart_t, uint8_t readFifoCapacity = 32> using PinPD0_t = UsartRxPin<Info::PinPD0Info, usart_t, readFifoCapacity>;

/**
 * Declares pin PD0 / RXD / PCINT16 / Arduino Digital 0, without USART support.
 */
constexpr PinPD0_t<NoUsart,32> PinPD0() {
    return PinPD0_t<NoUsart,32>();
}

/**
 * Declares pin PD0 / RXD / PCINT16 / Arduino Digital 0, with USART support. You'll need an instance of Usart0, as follows:
 * If to be used as RXD, declare as:
 *
 *     Usart0 usart0(57600);
 *     auto pinPD0 = PinPD0(usart0);
 *     mkISRS(pinPD0);
 *
 * In order to customize the read FIFO size, include a template parameter:
 *
 *     auto pinPD0 = PinPD0<32>(usart0);
 */
template <uint8_t readFifoCapacity = 64, typename usart_t>
constexpr PinPD0_t<usart_t, readFifoCapacity> PinPD0(usart_t &usart) {
    return PinPD0_t<usart_t, readFifoCapacity>();
}

template <typename usart_t, uint8_t writeFifoCapacity = 16> using PinPD1_t = UsartTxPin<Info::PinPD1Info, usart_t, writeFifoCapacity>;

/**
 * Declares pin PD1 / TXD / PCINT17 / Arduino Digital 1, without USART support.
 */
constexpr PinPD1_t<NoUsart,16> PinPD1() {
    return PinPD1_t<NoUsart,16>();
}

/**
 * Declares pin PD1 / TXD / PCINT17 / Arduino Digital 1, with USART support. You'll need an instance of Usart0, as follows:
 *
 *     Usart0 usart0(57600);
 *     auto pinPD1 = PinPD1(usart0);
 *     mkISRS(pinPD1);
 *
 * In order to customize the write FIFO size, include a template parameter:
 *
 *     auto pinPD1 = PinPD1<32>(usart0);
 */
template <uint8_t writeFifoCapacity = 64, typename usart_t>
constexpr PinPD1_t<usart_t, writeFifoCapacity> PinPD1(usart_t &usart) {
    return PinPD1_t<usart_t, writeFifoCapacity>();
}

/**
 * Declares pin PD2 / INT0 / PCINT18 / Arduino Digital 2.
 */
class PinPD2: public ExtInterruptPin<Info::PinPD2Info,Info::Int0Info> {};

template <typename timer2_t = NoTimer, class Enable=void>
class PinPD3_t {
    typename timer2_t::fail error_wrong_timer_template_argument;
};

template <typename timer2_t>
class PinPD3_t<timer2_t, typename std::enable_if<std::is_same<timer2_t, NoTimer>::value>::type>: public Pin<Info::PinPD3Info>, public ExtInterrupt<Info::Int1Info>  {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

/**
 * Declares pin PD3 / INT1 / OC2B / PCINT19 / Arduino Digital 3. This variant declares the pin without timer / PWM capability:
 *
 *     auto pinPD3 = PinPD3();
 */
constexpr PinPD3_t<NoTimer> PinPD3() {
    return PinPD3_t<NoTimer>();
}

template <typename timer2_t>
class PinPD3_t<timer2_t, typename std::enable_if<std::is_same<typename timer2_t::timer_info_t, typename Info::PinPD3Info::timer_info_t>::value>::type>:
         public PinOnComparatorB<Info::PinPD3Info,timer2_t>, public ExtInterrupt<Info::Int1Info> {
public:
    PinPD3_t(timer2_t &_timer): PinOnComparatorB<Info::PinPD3Info,timer2_t>(_timer) {}
};

/**
 * Declares pin PD3 / INT1 / OC2B / PCINT19 / Arduino Digital 3. This variant declares the pin with timer / PWM capability:
 *
 *     auto timer2 = Timer2().inNormalMode();
 *     auto pinPD3 = PinPD3(timer2);
 */
template <typename timer2_t>
constexpr PinPD3_t<timer2_t> PinPD3(timer2_t &timer) {
    return PinPD3_t<timer2_t>(timer);
}

/**
 * Declares pin PD4 / PCINT20 / XCK (TODO) / T0 (TODO) / Arduino Digital 4.
 */
typedef PinWithPinChangeOption<Info::PinPD4Info> PinPD4;

template <typename timer0_t>
class PinPD5_t: public WithPinChangeOption<PinOnComparatorB<Info::PinPD5Info,timer0_t>,Info::PinPD5Info> {
	typedef WithPinChangeOption<PinOnComparatorB<Info::PinPD5Info,timer0_t>,Info::PinPD5Info> Super;
public:
	using Super::Super;
};

/**
 * Declares pin PD5 / PCINT21 / OC0B / T1 (TODO) / Arduino Digital 5, without timer capability.
 */
constexpr PinPD5_t<NoTimer> PinPD5() {
    return PinPD5_t<NoTimer>();
}

/**
 * Declares pin PD5 / PCINT21 / OC0B / T1 (TODO) / Arduino Digital 5, with timer capability. You'll need a Timer0 instance:
 *
 *     auto timer0 = Timer0().inNormalMode();
 *     auto pinPD5 = PinPD5(timer0);
 */
template <typename timer0_t>
constexpr PinPD5_t<timer0_t> PinPD5(timer0_t &timer) {
    return PinPD5_t<timer0_t>(timer);
}

template <typename timer0_t>
class PinPD6_t: public WithPinChangeOption<PinOnComparatorB<Info::PinPD6Info,timer0_t>,Info::PinPD6Info> {
	typedef WithPinChangeOption<PinOnComparatorB<Info::PinPD6Info,timer0_t>,Info::PinPD6Info> Super;
public:
	using Super::Super;
};

/**
 * Declares pin PD6 / PCINT22 / OC0A / AIN0 (TODO) / Arduino Digital 6, without timer capability.
 */
constexpr PinPD6_t<NoTimer> PinPD6() {
    return PinPD6_t<NoTimer>();
}

/**
 * Declares pin PD6 / PCINT22 / OC0A / AIN0 (TODO) / Arduino Digital 6, with timer capability. You'll need a Timer0:
 *
 *     auto timer0 = Timer0().inNormalMode();
 *     auto pinPD6 = PinPD6(timer0);
 */
template <typename timer0_t>
constexpr PinPD6_t<timer0_t> PinPD6(timer0_t &timer) {
    return PinPD6_t<timer0_t>(timer);
}

/**
 * Declares pin PD7 / PCINT23 / AIN1 (TODO) / Arduino Digital 7.
 */
typedef PinWithPinChangeOption<Info::PinPD7Info> PinPD7;

/**
 * Declares pin PB0 / PCINT0 / CLKO (TODO) / ICP1 (TODO) / Arduino Digital 8.
 */
typedef PinWithPinChangeOption<Info::PinPB0Info> PinPB0;

template <typename timer1_t>
class PinPB1_t: public WithPinChangeOption<PinOnComparatorA<Info::PinPB1Info,timer1_t>,Info::PinPB1Info> {
	typedef WithPinChangeOption<PinOnComparatorA<Info::PinPB1Info,timer1_t>,Info::PinPB1Info> Super;
public:
	using Super::Super;
};

/**
 * Declares pin PB1 / PCINT1 / OC1A / Arduino Digital 9, without timer capability.
 */
constexpr PinPB1_t<NoTimer> PinPB1() {
    return PinPB1_t<NoTimer>();
}

/**
 * Declares pin PB1 / PCINT1 / OC1A / Arduino Digital 9, with timer capability. You'll need a Timer1:
 *
 *     auto timer1 = Timer1().inNormalMode();
 *     auto pinPB1 = PinPB1(timer1);
 */
template <typename timer1_t>
constexpr PinPB1_t<timer1_t> PinPB1(timer1_t &timer) {
    return PinPB1_t<timer1_t>(timer);
}

template <typename timer1_t>
class PinPB2_t: public WithPinChangeOption<PinOnComparatorB<Info::PinPB2Info,timer1_t>,Info::PinPB2Info> {
	typedef WithPinChangeOption<PinOnComparatorB<Info::PinPB2Info,timer1_t>,Info::PinPB2Info> Super;
public:
	using Super::Super;
};

/**
 * Declares pin PB2 / PCINT2 / SS / OC1B / Arduino Digital 10, without timer capability.
 */
constexpr PinPB2_t<NoTimer> PinPB2() {
    return PinPB2_t<NoTimer>();
}

/**
 * Declares pin PB2 / PCINT2 / SS / OC1B / Arduino Digital 10, with timer capability. You'll need a Timer1:
 *
 *     auto timer1 = Timer1().inNormalMode();
 *     auto pinPB2 = PinPB2(timer1);
 */
template <typename timer1_t>
constexpr PinPB2_t<timer1_t> PinPB2(timer1_t &timer) {
    return PinPB2_t<timer1_t>(timer);
}

template <typename timer2_t>
class PinPB3_t: public WithPinChangeOption<PinOnComparatorA<Info::PinPB3Info,timer2_t>,Info::PinPB3Info> {
	typedef WithPinChangeOption<PinOnComparatorA<Info::PinPB3Info,timer2_t>,Info::PinPB3Info> Super;
public:
	using Super::Super;
};

/**
 * Declares pin PB3 / PCINT3 / MOSI / OC2A / Arduino Digital 11, without timer capability.
 */
constexpr PinPB3_t<NoTimer> PinPB3() {
    return PinPB3_t<NoTimer>();
}

/**
 * Declares pin PB3 / PCINT3 / MOSI / OC2A / Arduino Digital 11, with timer capability. You'll need a Timer2:
 *
 *     auto timer1 = Timer1().inNormalMode();
 *     auto pinPB2 = PinPB2(timer1);
 */
template <typename timer2_t>
constexpr PinPB3_t<timer2_t> PinPB3(timer2_t &timer) {
    return PinPB3_t<timer2_t>(timer);
}

/**
 * Declares pin PB4 / PCINT4 / MISO / Arduino Digital 12.
 */
typedef PinWithPinChangeOption<Info::PinPB4Info> PinPB4;

/**
 * Declares pin PB5 / PCINT5 / SCK / Arduino Digital 13.
 */
typedef PinWithPinChangeOption<Info::PinPB5Info> PinPB5;

/**
 * Declares pin PC0 / ADC0 / PCINT8 / Arduino Analog 0.
 */
typedef PinWithPinChangeOption<Info::PinPC0Info> PinPC0;

/**
 * Declares pin PC1 / ADC1 / PCINT9 / Arduino Analog 1.
 */
typedef PinWithPinChangeOption<Info::PinPC1Info> PinPC1;

/**
 * Declares pin PC2 / ADC2 / PCINT10 / Arduino Analog 2.
 */
typedef PinWithPinChangeOption<Info::PinPC2Info> PinPC2;

/**
 * Declares pin PC3 / ADC3 / PCINT11 / Arduino Analog 3.
 */
typedef PinWithPinChangeOption<Info::PinPC3Info> PinPC3;

/**
 * Declares pin PC4 / ADC4 / PCINT12 / SDA / Arduino Analog 4.
 */
typedef PinWithPinChangeOption<Info::PinPC4Info> PinPC4;

/**
 * Declares pin PC5 / ADC5 / PCINT13 / SCL / Arduino Analog 5.
 */
typedef PinWithPinChangeOption<Info::PinPC5Info> PinPC5;

/**
 * Declares pin ADC6 / Arduino Analog 6.
 */
typedef ADCOnlyPin<Info::PinA6Info> PinADC6;

/**
 * Declares pin ADC7 / Arduino Analog 7.
 */
typedef ADCOnlyPin<Info::PinA7Info> PinADC7;

struct TWIInfo {
    typedef Info::PinPC4Info PinSDA;
    typedef Info::PinPC5Info PinSCL;
};

template <uint8_t txFifoSize = 32, uint8_t rxFifoSize = 32, uint32_t twiFreq = 100000l>
using TWI = Impl::TWI<TWIInfo, txFifoSize, rxFifoSize, twiFreq>;


} // namespace Atmel
} // namespace HAL

/**
 * Defines all ISRS that exist on THIS chip.
 */
#define mkISRS \
    mkISR(INT0, 1) \
    mkISR(INT1, 2) \
    mkISR(PCINT0, 3) \
    mkISR(PCINT1, 4) \
    mkISR(PCINT2, 5) \
    mkISR(WDT, 6) \
    mkISR(TIMER2_COMPA, 7) \
    mkISR(TIMER2_COMPB, 8) \
    mkISR(TIMER2_OVF, 9) \
    mkISR(TIMER1_COMPA, 11) \
    mkISR(TIMER1_COMPB, 12) \
    mkISR(TIMER1_OVF, 13) \
    mkISR(TIMER0_COMPA, 14) \
    mkISR(TIMER0_COMPB, 15) \
    mkISR(TIMER0_OVF, 16) \
    mkISR(USART_RX, 18) \
    mkISR(USART_UDRE, 19) \
    mkISR(ADC, 21) \
    mkISR(TWI, 24) \

/*
//    mkISR(TIMER1_CAPT, 10) \
//    mkISR(SPI_STC, 17) \
//    mkISR(USART_TX, 20) \
//mkISR(EE_READY, 22) \
//mkISR(ANALOG_COMP, 23) \
//    mkISR(SPM_READY, 25) \
*/

// --------------------------------- Arduino stuff -------------------------------------

#define ArduinoPinD0 ::HAL::Atmel::PinPD0
#define ArduinoPinD1 ::HAL::Atmel::PinPD1
#define ArduinoPinD2 ::HAL::Atmel::PinPD2
#define ArduinoPinD3 ::HAL::Atmel::PinPD3
#define ArduinoPinD4 ::HAL::Atmel::PinPD4
#define ArduinoPinD5 ::HAL::Atmel::PinPD5
#define ArduinoPinD6 ::HAL::Atmel::PinPD6
#define ArduinoPinD7 ::HAL::Atmel::PinPD7
#define ArduinoPinD8 ::HAL::Atmel::PinPB0
#define ArduinoPinD9 ::HAL::Atmel::PinPB1
#define ArduinoPinD10 ::HAL::Atmel::PinPB2
#define ArduinoPinD11 ::HAL::Atmel::PinPB3
#define ArduinoPinD12 ::HAL::Atmel::PinPB4
#define ArduinoPinD13 ::HAL::Atmel::PinPB5
#define ArduinoPinA0 ::HAL::Atmel::PinPC0
#define ArduinoPinA1 ::HAL::Atmel::PinPC1
#define ArduinoPinA2 ::HAL::Atmel::PinPC2
#define ArduinoPinA3 ::HAL::Atmel::PinPC3
#define ArduinoPinA4 ::HAL::Atmel::PinPC4
#define ArduinoPinA5 ::HAL::Atmel::PinPC5
#define ArduinoPinA6 ::HAL::Atmel::PinADC6
#define ArduinoPinA7 ::HAL::Atmel::PinADC7

// --------------------------------- JeeNode stuff -------------------------------------

/**
 * Declares the shared "I" Interrupt pin on all JeeNode ports
 */
#define JeeNodePortI ::HAL::Atmel::PinPD3

/**
 * Declares JeeNode Port 1 Analog
 */
#define JeeNodePort1A ::HAL::Atmel::PinPC0

/**
 * Declares JeeNode Port 2 Analog
 */
#define JeeNodePort2A ::HAL::Atmel::PinPC1

/**
 * Declares JeeNode Port 3 Analog
 */
#define JeeNodePort3A ::HAL::Atmel::PinPC2

/**
 * Declares JeeNode Port 1 Analog
 */
#define JeeNodePort4A ::HAL::Atmel::PinPC3

/**
 * Declares JeeNode Port 1 Digital
 */
#define JeeNodePort1D ::HAL::Atmel::PinPD4

/**
 * Declares JeeNode Port 2 Digital
 */
#define JeeNodePort2D ::HAL::Atmel::PinPD5

/**
 * Declares JeeNode Port 3 Digital
 */
#define JeeNodePort3D ::HAL::Atmel::PinPD6

/**
 * Declares JeeNode Port 1 Digital
 */
#define JeeNodePort4D ::HAL::Atmel::PinPD7

#endif /* HAL_ATMEL_DEVICES_ATMEGA328P_HPP_ */
