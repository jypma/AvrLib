#ifndef HAL_ATMEL_DEVICES_ATMEGA328_HPP_
#define HAL_ATMEL_DEVICES_ATMEGA328_HPP_

#include "Time/Prescaled.hpp"
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

enum class IntPrescaler: uint8_t {
    _1 = _BV(CS00),
    _8 = _BV(CS01),
    _32 = _BV(CS00) | _BV(CS01),
    _64 = _BV(CS02),
    _128 = _BV(CS02) | _BV(CS00),
    _256 = _BV(CS02) | _BV(CS01),
    _1024 = _BV(CS02) | _BV(CS01) | _BV(CS00)
};

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
        static constexpr volatile uint16_t *ocr = &OCR1A;
        static constexpr uint8_t timsk_bit = OCIE1A;
        static constexpr uint8_t tifr_bit = OCF1A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM1A0) | (1 << COM1A1);
        static constexpr uint8_t output_mode_bitstart = COM1A0;
        static constexpr uint8_t foc = FOC1A;
    };
    struct ComparatorB: public Comparator {
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
        static constexpr volatile uint8_t *ocr = &OCR2A;
        static constexpr uint8_t timsk_bit = OCIE2A;
        static constexpr uint8_t tifr_bit = OCF2A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM2A0) | (1 << COM2A1);
        static constexpr uint8_t output_mode_bitstart = COM2A0;
        static constexpr uint8_t foc = FOC2A;
    };
    struct ComparatorB: public Comparator {
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

struct PinA0Info: public PinOnPortC<0>, public GPIOPin {
    static constexpr uint8_t adc_mux = 0;
};
struct PinA1Info: public PinOnPortC<1>, public GPIOPin {
    static constexpr uint8_t adc_mux = 1;
};
struct PinA2Info: public PinOnPortC<2>, public GPIOPin {
    static constexpr uint8_t adc_mux = 2;
};
struct PinA3Info: public PinOnPortC<3>, public GPIOPin {
    static constexpr uint8_t adc_mux = 3;
};
struct PinA4Info: public PinOnPortC<4>, public GPIOPin {
    static constexpr uint8_t adc_mux = 4;
};
struct PinA5Info: public PinOnPortC<5>, public GPIOPin {
    static constexpr uint8_t adc_mux = 5;
};
struct PinA6Info {
    static constexpr uint8_t adc_mux = 6;
};
struct PinA7Info {
    static constexpr uint8_t adc_mux = 7;
};

} // namespace Info

////////////////////////////// TIMERS ///////////////////////////////////////////////////////

template <ExtPrescaler prescaler> using Timer0_Normal = Timer::NormalTimer<Info::Timer0Info,prescaler>;
template <ExtPrescaler prescaler> using Timer1_Normal = Timer::NormalTimer<Info::Timer1Info,prescaler>;
template <IntPrescaler prescaler> using Timer2_Normal = Timer::NormalTimer<Info::Timer2Info,prescaler>;
template <ExtPrescaler prescaler> using Timer0_FastPWM = Timer::FastPWMTimer<Info::Timer0Info,prescaler>;
template <ExtPrescaler prescaler> using Timer1_FastPWM = Timer::FastPWMTimer<Info::Timer1Info,prescaler>;
template <IntPrescaler prescaler> using Timer2_FastPWM = Timer::FastPWMTimer<Info::Timer2Info,prescaler>;

////////////////////////////// PINS /////////////////////////////////////////////////////////
/**
 * Declares Usart0 to be used.
 * In order to actually send or receive data, declare types and instances of PinPD1 or PinPD0.
 * Pass the baud rate as constructor argument, e.g.
 *
 *     Usart0 usart0(57600)
 */
typedef Usart<Info::Usart0Info> Usart0;

/**
 * Declares pin PD0 / RXD / PCINT16 / Arduino Digital 0.
 * If to be used as RXD, declare as:
 *
 *     Usart0 usart0(57600);
 *     PinPD0<Usart0> pinPD0(usart0);
 *     ISR_USART_RX(pinPD0);
 *
 * If to be used without USART, just say:
 *
 *     PinPD0<> pinPD0;
 */
template <typename usart_t = NoUsart, uint8_t readFifoCapacity = 32> using PinPD0 = UsartRxPin<Info::PinPD0Info, usart_t, readFifoCapacity>;

/**
 * Declares pin PD1 / TXD / PCINT17 / Arduino Digital 1.
 * If to be used as TXD, declare as:
 *
 *     Usart0 usart0(57600);
 *     PinPD0<Usart1> pinPD1(usart0);
 *     ISR_USART_UDRE(pinPD0);
 *
 * If to be used without USART, just say:
 *
 *     PinD0<> pinD0;
 */
template <typename usart_t = NoUsart, uint8_t writeFifoCapacity = 16> using PinPD1 = UsartTxPin<Info::PinPD1Info, usart_t, writeFifoCapacity>;

/**
 * Declares pin PD2 / INT0 / PCINT18 / Arduino Digital 2.
 */
class PinPD2: public ExtInterruptPin<Info::PinPD2Info,Info::Int0Info> {};

template <typename timer2_t = NoTimer, class Enable=void>
class PinPD3 {
    typename timer2_t::fail error_wrong_timer_template_argument;
};

/**
 * Declares pin PD3 / INT1 / OC2B / PCINT19 / Arduino Digital 3. This variant declares the pin without timer / PWM capability:
 *
 *     auto pinPD3 = PinPD3();
 */
template <typename timer2_t>
class PinPD3<timer2_t, typename std::enable_if<std::is_same<timer2_t, NoTimer>::value>::type>: public Pin<Info::PinPD3Info>, public ExtInterrupt<Info::Int1Info>  {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

/**
 * Declares pin PD3 / INT1 / OC2B / PCINT19 / Arduino Digital 3. This variant declares the pin with timer / PWM capability:
 *
 *     auto timer2 = Timer2().inNormalMode();
 *     auto pinPD3 = PinPD3(timer2);
 */
template <typename timer2_t>
class PinPD3<timer2_t, typename std::enable_if<std::is_same<typename timer2_t::timer_info_t, typename Info::PinPD3Info::timer_info_t>::value>::type>:
         public PinOnComparatorB<Info::PinPD3Info,timer2_t>, public ExtInterrupt<Info::Int1Info> {
public:
    PinPD3(timer2_t &_timer): PinOnComparatorB<Info::PinPD3Info,timer2_t>(_timer) {}
};

/**
 * Declares pin PD4 / PCINT20 / XCK (TODO) / T0 (TODO) / Arduino Digital 4.
 */
typedef Pin<Info::PinPD4Info> PinPD4;

/**
 * Declares pin PD5 / PCINT21 / OC0B / T1 (TODO) / Arduino Digital 5.
 * In order to use the pin without timer capability:
 *
 *     auto pinPD5 = PinPD5();
 *
 * In order to use the pin with timer capability:
 *
 *     auto timer0 = Timer0().inNormalMode();
 *     auto pinPD5 = PinPD5(timer0);
 */
template <typename timer0_t = NoTimer> using PinPD5 = PinOnComparatorB<Info::PinPD5Info,timer0_t>;

/**
 * Declares pin PD6 / PCINT22 / OC0A / AIN0 (TODO) / Arduino Digital 6.
 * In order to use the pin without timer capability:
 *
 *     auto pinPD6 = PinPD6();
 *
 * In order to use the pin with timer capability:
 *
 *     auto timer0 = Timer0().inNormalMode();
 *     auto pinPD6 = PinPD6(timer0);
 */
template <typename timer0_t = NoTimer> using PinPD6 = PinOnComparatorA<Info::PinPD6Info,timer0_t>;

/**
 * Declares pin PD7 / PCINT23 / AIN1 (TODO) / Arduino Digital 7.
 */
typedef Pin<Info::PinPD7Info> PinPD7;

/**
 * Declares pin PB0 / PCINT0 / CLKO (TODO) / ICP1 (TODO) / Arduino Digital 8.
 */
typedef Pin<Info::PinPB0Info> PinPB0;

/**
 * Declares pin PB1 / PCINT1 / OC1A / Arduino Digital 9.
 * In order to use the pin without timer capability:
 *
 *     auto pinPB1 = PinPB1();
 *
 * In order to use the pin with timer capability:
 *
 *     auto timer1 = Timer1().inNormalMode();
 *     auto pinPB1 = PinPB1(timer1);
 */
template <typename timer1_t = NoTimer> using PinPB1 = PinOnComparatorA<Info::PinPB1Info,timer1_t>;

/**
 * Declares pin PB2 / PCINT2 / SS / OC1B / Arduino Digital 10.
 * In order to use the pin without timer capability:
 *
 *     auto pinPB2 = PinPB2();
 *
 * In order to use the pin with timer capability:
 *
 *     auto timer1 = Timer1().inNormalMode();
 *     auto pinPB2 = PinPB2(timer1);
 */
template <typename timer1_t = NoTimer> using PinPB2 = PinOnComparatorB<Info::PinPB2Info,timer1_t>;

/**
 * Declares pin PB3 / PCINT3 / MOSI / OC2A / Arduino Digital 11.
 * In order to use the pin without timer capability:
 *
 *     auto pinPB3 = PinPB3();
 *
 * In order to use the pin with timer capability:
 *
 *     auto timer2 = Timer2().inNormalMode();
 *     auto pinPB3 = PinPB3(timer1);
 */
template <typename timer2_t = NoTimer> using PinPB3 = PinOnComparatorA<Info::PinPB3Info,timer2_t>;

/**
 * Declares pin PB4 / PCINT4 / MISO / Arduino Digital 12.
 */
typedef Pin<Info::PinPB4Info> PinPB4;

/**
 * Declares pin PB5 / PCINT5 / SCK / Arduino Digital 13.
 */
typedef Pin<Info::PinPB5Info> PinPB5;


typedef Pin<Info::PinA0Info> PinA0;
typedef Pin<Info::PinA1Info> PinA1;
typedef Pin<Info::PinA2Info> PinA2;
typedef Pin<Info::PinA3Info> PinA3;
typedef Pin<Info::PinA4Info> PinA4;
typedef Pin<Info::PinA5Info> PinA5;
typedef ADCOnlyPin<Info::PinA6Info> PinA6;
typedef ADCOnlyPin<Info::PinA7Info> PinA7;

} // namespace Atmel
} // namespace HAL

/**
 * Defines all ISRS that exist on THIS chip.
 */
#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, INT0_, INT1_, TIMER0_OVF_, TIMER0_COMPA_, TIMER0_COMPB_, TIMER1_OVF_, TIMER1_COMPA_, TIMER1_COMPB_, TIMER2_OVF_, TIMER2_COMPA_, TIMER2_COMPB_, USART_RX_, USART_UDRE_)

#endif /* HAL_ATMEL_DEVICES_ATMEGA328_HPP_ */
