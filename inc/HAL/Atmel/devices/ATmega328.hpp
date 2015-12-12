#ifndef HAL_ATMEL_DEVICES_ATMEGA328_HPP_
#define HAL_ATMEL_DEVICES_ATMEGA328_HPP_

#include "Time/Prescaled.hpp"
#include "HAL/Atmel/Usart.hpp"
#include "HAL/Atmel/Pin.hpp"
#include "HAL/Atmel/InterruptVectors.hpp"

namespace HAL {
namespace Atmel {
namespace Info {

struct Int0Info {
    typedef INTERRUPT_VECTOR(INT0) INT;
    inline static void on(uint8_t mode) {
        EICRA = (EICRA & ~(_BV(ISC00) | _BV(ISC01))) | (mode << ISC00);
        EIMSK |= _BV(INT0);
    }
    inline static void off() {
        EIMSK &= ~_BV(INT0);
    }
};

struct Int1Info {
    typedef INTERRUPT_VECTOR(INT1) INT;
    inline static void on(uint8_t mode) {
        EICRA = (EICRA & ~(_BV(ISC10) | _BV(ISC11))) | (mode << ISC10);
        EIMSK |= _BV(INT1);
    }
    inline static void off() {
        EIMSK &= ~_BV(INT1);
    }
};

struct PCInt0Info {
    typedef INTERRUPT_VECTOR(PCINT0) PCINT;
    static constexpr uint8_t PCIE = PCIE0;
    static constexpr volatile uint8_t *pin = &PINB;
    static constexpr volatile uint8_t *pcmsk = &PCMSK0;
};

struct PCInt1Info {
    typedef INTERRUPT_VECTOR(PCINT1) PCINT;
    static constexpr uint8_t PCIE = PCIE1;
    static constexpr volatile uint8_t *pin = &PINC;
    static constexpr volatile uint8_t *pcmsk = &PCMSK1;
};

struct PCInt2Info {
    typedef INTERRUPT_VECTOR(PCINT2) PCINT;
    static constexpr uint8_t PCIE = PCIE2;
    static constexpr volatile uint8_t *pin = &PIND;
    static constexpr volatile uint8_t *pcmsk = &PCMSK2;
};

struct Usart0Info {
    static constexpr volatile uint8_t *ucsra = &UCSR0A;
    static constexpr volatile uint8_t *ucsrb = &UCSR0B;
    static constexpr volatile uint8_t *ucsrc = &UCSR0C;
    static constexpr volatile uint16_t *ubrr = &UBRR0;
    static constexpr volatile uint8_t *udr = &UDR0;
};

} // namespace Info

enum class ExtPrescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _64 = _BV(CS00) | _BV(CS01),
    _256 = _BV(CS02),
    _1024 = _BV(CS02) | _BV(CS00)
};

template <uint16_t i>
constexpr static ExtPrescaler extPrescalerFromInt() {
    static_assert(i == 1 || i == 8 || i == 64 || i == 256 || i == 1024, "prescaler must be 1, 8, 64, 256 or 1024.");
    if (i == 1) {
        return ExtPrescaler::_1;
    } else if (i == 8) {
        return ExtPrescaler::_8;
    } else if (i == 64) {
        return ExtPrescaler::_64;
    } else if (i == 256) {
        return ExtPrescaler::_256;
    } else if (i == 1024) {
        return ExtPrescaler::_1024;
    }
}

enum class IntPrescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _32 = _BV(CS00) | _BV(CS01),
    _64 = _BV(CS02),
    _128 = _BV(CS02) | _BV(CS00),
    _256 = _BV(CS02) | _BV(CS01),
    _1024 = _BV(CS02) | _BV(CS01) | _BV(CS00)
};

template <uint16_t i>
constexpr static IntPrescaler intPrescalerFromInt() {
    static_assert(i == 1 || i == 8 || i == 32 || i == 64 || i == 128 || i == 256 || i == 1024, "prescaler must be 1, 8, 32, 64, 128, 256 or 1024.");
    if (i == 1) {
        return IntPrescaler::_1;
    } else if (i == 8) {
        return IntPrescaler::_8;
    } else if (i == 32) {
        return IntPrescaler::_32;
    } else if (i == 64) {
        return IntPrescaler::_64;
    } else if (i == 128) {
        return IntPrescaler::_128;
    } else if (i == 256) {
        return IntPrescaler::_256;
    } else if (i == 1024) {
        return IntPrescaler::_1024;
    }
}

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
    typedef INTERRUPT_VECTOR(TIMER0_OVF) INT;

    static constexpr volatile uint8_t *tccra = &TCCR0A;
    static constexpr volatile uint8_t *tccrb = &TCCR0B;
    static constexpr volatile uint8_t *tcnt = &TCNT0;
    static constexpr volatile uint8_t *timsk = &TIMSK0;
    static constexpr volatile uint8_t *tifr = &TIFR0;

    typedef uint8_t value_t;
    typedef ExtPrescaler prescaler_t;
    template <uint16_t i> constexpr static ExtPrescaler prescalerFromInt() { return extPrescalerFromInt<i>(); }

    inline static void configureNormal(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra &= ~_BV(WGM00);
        *tccra &= ~_BV(WGM01); // Mode 0, normal
        *tccrb = static_cast<uint8_t>(p);
    }

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra |= _BV(WGM00);
        *tccra |= _BV(WGM01); // Mode 3, count up to 0xFF
        *tccrb = static_cast<uint8_t>(p);
    }

    struct Comparator {
        typedef uint8_t value_t;
        typedef Timer0Info timer_info_t;
        static constexpr volatile uint8_t *tcnt = &TCNT0;
        static constexpr volatile uint8_t *timsk = &TIMSK0;
        static constexpr volatile uint8_t *tifr = &TIFR0;
        static constexpr volatile uint8_t *tccra = &TCCR0A;
        static constexpr volatile uint8_t *tccrb = &TCCR0B;
    };

    struct ComparatorA: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER0_COMPA) INT;
        static constexpr volatile uint8_t *ocr = &OCR0A;
        static constexpr uint8_t timsk_bit = OCIE0A;
        static constexpr uint8_t tifr_bit = OCF0A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM0A0) | (1 << COM0A1);
        static constexpr uint8_t output_mode_bitstart = COM0A0;
        static constexpr uint8_t foc = FOC0A;
    };
    struct ComparatorB: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER0_COMPB) INT;
        static constexpr volatile uint8_t *ocr = &OCR0B;
        static constexpr uint8_t timsk_bit = OCIE0B;
        static constexpr uint8_t tifr_bit = OCF0B;
        static constexpr uint8_t output_mode_bitmask = (1 << COM0B0) | (1 << COM0B1);
        static constexpr uint8_t output_mode_bitstart = COM0B0;
        static constexpr uint8_t foc = FOC0B;
    };
};

struct Timer1Info {
    typedef INTERRUPT_VECTOR(TIMER1_OVF) INT;

    static constexpr volatile uint8_t *tccra = &TCCR1A;
    static constexpr volatile uint8_t *tccrb = &TCCR1B;
    static constexpr volatile uint16_t *tcnt = &TCNT1;
    static constexpr volatile uint8_t *timsk = &TIMSK1;
    static constexpr volatile uint8_t *tifr = &TIFR1;

    typedef uint16_t value_t;
    typedef ExtPrescaler prescaler_t;
    template <uint16_t i> constexpr static ExtPrescaler prescalerFromInt() { return extPrescalerFromInt<i>(); }

    inline static void configureNormal(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra &= ~_BV(WGM10);
        *tccra &= ~_BV(WGM11);
        *tccrb = static_cast<uint8_t>(p);
    }

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra &= ~_BV(WGM10);
        *tccra |= _BV(WGM11);
        *tccrb = _BV(WGM13) | _BV(WGM12) | static_cast<uint8_t>(p);
        ICR1 = 0xFFFF; // Mode 14, count up to ICR1, which we always set at 0xFFFF.
    }

    struct Comparator {
        typedef uint16_t value_t;
        typedef Timer1Info timer_info_t;
        static constexpr volatile uint16_t *tcnt = &TCNT1;
        static constexpr volatile uint8_t *timsk = &TIMSK1;
        static constexpr volatile uint8_t *tifr = &TIFR1;
        static constexpr volatile uint8_t *tccra = &TCCR1A;
        static constexpr volatile uint8_t *tccrb = &TCCR1B;
    };

    struct ComparatorA: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER1_COMPA) INT;
        static constexpr volatile uint16_t *ocr = &OCR1A;
        static constexpr uint8_t timsk_bit = OCIE1A;
        static constexpr uint8_t tifr_bit = OCF1A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM1A0) | (1 << COM1A1);
        static constexpr uint8_t output_mode_bitstart = COM1A0;
        static constexpr uint8_t foc = FOC1A;
    };
    struct ComparatorB: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER1_COMPA) INT;
        static constexpr volatile uint16_t *ocr = &OCR1B;
        static constexpr uint8_t timsk_bit = OCIE1B;
        static constexpr uint8_t tifr_bit = OCF1B;
        static constexpr uint8_t output_mode_bitmask = (1 << COM1B0) | (1 << COM1B1);
        static constexpr uint8_t output_mode_bitstart = COM1B0;
        static constexpr uint8_t foc = FOC1B;
    };
};

struct Timer2Info {
    typedef INTERRUPT_VECTOR(TIMER2_OVF) INT;

    static constexpr volatile uint8_t *tccra = &TCCR2A;
    static constexpr volatile uint8_t *tccrb = &TCCR2B;
    static constexpr volatile uint8_t *tcnt = &TCNT2;
    static constexpr volatile uint8_t *timsk = &TIMSK2;
    static constexpr volatile uint8_t *tifr = &TIFR2;

    typedef uint8_t value_t;
    typedef IntPrescaler prescaler_t;
    template <uint16_t i> constexpr static IntPrescaler prescalerFromInt() { return intPrescalerFromInt<i>(); }

    inline static void configureNormal(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra &= ~_BV(WGM20);
        *tccra &= ~_BV(WGM21); // Mode 0, normal
        *tccrb = static_cast<uint8_t>(p);
    }

    inline static void configureFastPWM(prescaler_t p) {
        // Existing settings for COM0A1 etc. should be kept
        AtomicScope::SEI _;
        *tccra |= _BV(WGM20);
        *tccra |= _BV(WGM21); // Mode 3, count up to 0xFF
        *tccrb = static_cast<uint8_t>(p);
    }

    struct Comparator {
        typedef uint8_t value_t;
        typedef Timer2Info timer_info_t;
        static constexpr volatile uint8_t *tcnt = &TCNT2;
        static constexpr volatile uint8_t *timsk = &TIMSK2;
        static constexpr volatile uint8_t *tifr = &TIFR2;
        static constexpr volatile uint8_t *tccra = &TCCR2A;
        static constexpr volatile uint8_t *tccrb = &TCCR2B;
    };

    struct ComparatorA: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER2_COMPA) INT;
        static constexpr volatile uint8_t *ocr = &OCR2A;
        static constexpr uint8_t timsk_bit = OCIE2A;
        static constexpr uint8_t tifr_bit = OCF2A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM2A0) | (1 << COM2A1);
        static constexpr uint8_t output_mode_bitstart = COM2A0;
        static constexpr uint8_t foc = FOC2A;
    };
    struct ComparatorB: public Comparator {
        typedef INTERRUPT_VECTOR(TIMER2_COMPA) INT;
        static constexpr volatile uint8_t *ocr = &OCR2B;
        static constexpr uint8_t timsk_bit = OCIE2B;
        static constexpr uint8_t tifr_bit = OCF2B;
        static constexpr uint8_t output_mode_bitmask = (1 << COM2B0) | (1 << COM2B1);
        static constexpr uint8_t output_mode_bitstart = COM2B0;
        static constexpr uint8_t foc = FOC2B;
    };
};

template <uint8_t bit>
struct PinOnPortD {
    static constexpr volatile uint8_t *ddr = &DDRD;
    static constexpr volatile uint8_t *port = &PORTD;
    static constexpr volatile uint8_t *pin = &PIND;
    static constexpr uint8_t bitmask = _BV(bit);
};

template <uint8_t bit>
struct PinOnPortB {
    static constexpr volatile uint8_t *ddr = &DDRB;
    static constexpr volatile uint8_t *port = &PORTB;
    static constexpr volatile uint8_t *pin = &PINB;
    static constexpr uint8_t bitmask = _BV(bit);
};

template <uint8_t bit>
struct PinOnPortC {
    static constexpr volatile uint8_t *ddr = &DDRC;
    static constexpr volatile uint8_t *port = &PORTC;
    static constexpr volatile uint8_t *pin = &PINC;
    static constexpr volatile uint8_t *pcmsk = &PCMSK1;
    static constexpr uint8_t bitmask = _BV(bit);
    typedef Info::PCInt1Info pcintInfo;
};

struct PinPD0Info: public PinOnPortD<0> {
    typedef Usart0Info usart_info_t;

    static inline void configureAsGPIO() {
        UCSR0B &= ~_BV(RXEN0); // disable hardware USART receiver
    }
};

struct PinPD1Info: public PinOnPortD<1> {
    typedef Usart0Info usart_info_t;

    static inline void configureAsGPIO() {
        UCSR0B &= ~_BV(TXEN0); // disable hardware USART transmitter
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
    static constexpr uint8_t adc_mux = 0;
    static constexpr uint8_t pcintBit = 0;
};
struct PinPC1Info: public PinOnPortC<1>, public GPIOPin {
    static constexpr uint8_t adc_mux = 1;
    static constexpr uint8_t pcintBit = 1;
};
struct PinPC2Info: public PinOnPortC<2>, public GPIOPin {
    static constexpr uint8_t adc_mux = 2;
    static constexpr uint8_t pcintBit = 2;
};
struct PinPC3Info: public PinOnPortC<3>, public GPIOPin {
    static constexpr uint8_t adc_mux = 3;
    static constexpr uint8_t pcintBit = 3;
};
struct PinPC4Info: public PinOnPortC<4>, public GPIOPin {
    static constexpr uint8_t adc_mux = 4;
    static constexpr uint8_t pcintBit = 4;
};
struct PinPC5Info: public PinOnPortC<5>, public GPIOPin {
    static constexpr uint8_t adc_mux = 5;
    static constexpr uint8_t pcintBit = 5;
};
struct PinA6Info {
    static constexpr uint8_t adc_mux = 6;
};
struct PinA7Info {
    static constexpr uint8_t adc_mux = 7;
};

} // namespace Info

////////////////////////////// TIMERS ///////////////////////////////////////////////////////

typedef Timer::TimerDeclaration<Info::Timer0Info,ExtPrescaler::_1> Timer0;
typedef Timer::TimerDeclaration<Info::Timer1Info,ExtPrescaler::_1> Timer1;
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
template <uint8_t readFifoCapacity = 32, typename usart_t>
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
template <uint8_t writeFifoCapacity = 16, typename usart_t>
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
typedef Pin<Info::PinPD4Info> PinPD4;

template <typename timer0_t = NoTimer> using PinPD5_t = PinOnComparatorB<Info::PinPD5Info,timer0_t>;

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


template <typename timer0_t = NoTimer> using PinPD6_t = PinOnComparatorA<Info::PinPD6Info,timer0_t>;

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
typedef Pin<Info::PinPD7Info> PinPD7;

/**
 * Declares pin PB0 / PCINT0 / CLKO (TODO) / ICP1 (TODO) / Arduino Digital 8.
 */
typedef Pin<Info::PinPB0Info> PinPB0;

template <typename timer1_t = NoTimer> using PinPB1_t = PinOnComparatorA<Info::PinPB1Info,timer1_t>;

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

template <typename timer1_t = NoTimer> using PinPB2_t = PinOnComparatorB<Info::PinPB2Info,timer1_t>;

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

template <typename timer2_t = NoTimer> using PinPB3_t = PinOnComparatorA<Info::PinPB3Info,timer2_t>;

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
typedef Pin<Info::PinPB4Info> PinPB4;

/**
 * Declares pin PB5 / PCINT5 / SCK / Arduino Digital 13.
 */
typedef Pin<Info::PinPB5Info> PinPB5;

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
 * Declares pin PC4 / ADC4 / PCINT12 / Arduino Analog 4.
 */
typedef PinWithPinChangeOption<Info::PinPC4Info> PinPC4;

/**
 * Declares pin PC5 / ADC5 / PCINT13 / Arduino Analog 5.
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

} // namespace Atmel
} // namespace HAL

/**
 * Defines all ISRS that exist on THIS chip.
 */
#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, INT0_, INT1_, TIMER0_OVF_, TIMER0_COMPA_, TIMER0_COMPB_, TIMER1_OVF_, TIMER1_COMPA_, TIMER1_COMPB_, TIMER2_OVF_, TIMER2_COMPA_, TIMER2_COMPB_, USART_RX_, USART_UDRE_, PCINT0_, PCINT1_, PCINT2_)

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

#endif /* HAL_ATMEL_DEVICES_ATMEGA328_HPP_ */
