#ifndef HAL_ATMEL_DEVICES_ATMEGA328_HPP_
#define HAL_ATMEL_DEVICES_ATMEGA328_HPP_

#include "Time/Prescaled.hpp"
#include "HAL/Atmel/Pin.hpp"
#include "HAL/Atmel/ExternalInterrupt.hpp"
#include "HAL/Atmel/InterruptVectors.hpp"
#include "HAL/Atmel/Timer.hpp"

namespace HAL {
namespace Atmel {
namespace Info {

struct Int0Info {
    typedef HAL::Atmel::InterruptVectors::Vector_INT0 INT;
    inline static void on(uint8_t mode) {
        EICRA = (EICRA & ~(_BV(ISC00) | _BV(ISC01))) | (mode << ISC00);
        EIMSK |= _BV(INT0);
    }
    inline static void off() {
        EIMSK &= ~_BV(INT0);
    }
};

struct Int1Info {
    typedef HAL::Atmel::InterruptVectors::Vector_INT0 INT;
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
        static constexpr volatile uint8_t *ocr = &OCR0A;
        static constexpr uint8_t timsk_bit = OCIE0A;
        static constexpr uint8_t tifr_bit = OCF0A;
        static constexpr uint8_t output_mode_bitmask = (1 << COM0A0) | (1 << COM0A1);
        static constexpr uint8_t output_mode_bitstart = COM0A0;
        static constexpr uint8_t foc = FOC0A;
    };
    struct ComparatorB: public Comparator {
        static constexpr volatile uint8_t *ocr = &OCR0B;
        static constexpr uint8_t timsk_bit = OCIE0B;
        static constexpr uint8_t tifr_bit = OCF0B;
        static constexpr uint8_t output_mode_bitmask = (1 << COM0B0) | (1 << COM0B1);
        static constexpr uint8_t output_mode_bitstart = COM0B0;
        static constexpr uint8_t foc = FOC0B;
    };
};

struct Timer1Info {
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

template <typename pinInfo, typename extInterruptInfo>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo> {};

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


struct PinD2Info: public PinOnPortD<2>, public GPIOPin {};
struct PinD3Info: public PinOnPortD<3>, public GPIOPin, public PinOnTimer2 {};
struct PinD4Info: public PinOnPortD<4>, public GPIOPin {};
struct PinD5Info: public PinOnPortD<5>, public GPIOPin, public PinOnTimer0 {};
struct PinD6Info: public PinOnPortD<6>, public GPIOPin, public PinOnTimer0 {};
struct PinD7Info: public PinOnPortD<7>, public GPIOPin {};
struct PinD8Info: public PinOnPortB<0>, public GPIOPin {};
struct PinD9Info: public PinOnPortB<1>, public GPIOPin, public PinOnTimer1 {};
struct PinD10Info: public PinOnPortB<2>, public GPIOPin, public PinOnTimer1 {};
struct PinD11Info: public PinOnPortB<3>, public GPIOPin, public PinOnTimer2 {};
struct PinD12Info: public PinOnPortB<4>, public GPIOPin {};
struct PinD13Info: public PinOnPortB<5>, public GPIOPin {};

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
class PinPD2: public Info::ExtInterruptPin<Info::PinD2Info,Info::Int0Info> {};

} // namespace Atmel
} // namespace HAL

#endif /* HAL_ATMEL_DEVICES_ATMEGA328_HPP_ */
