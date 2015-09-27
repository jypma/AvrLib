#ifndef HAL_ATMEL_PIN_HPP_
#define HAL_ATMEL_PIN_HPP_

#include <HAL/Atmel/Usart.hpp>
#include "HAL/Atmel/ExternalInterrupt.hpp"
#include "HAL/Atmel/Timer.hpp"

namespace HAL {
namespace Atmel {

template <typename info>
class Pin {
public:
    typedef info info_t;

    void configureAsOutput() const {
        info::configureAsGPIO();
        *info::ddr |= info::bitmask;
    }

    void configureAsInputWithoutPullup() const {
        info::configureAsGPIO();
        *info::ddr &= ~info::bitmask;
        *info::port &= ~info::bitmask;
    }
    void configureAsInputWithPullup() const {
        *info::ddr &= ~info::bitmask;
        *info::port |= info::bitmask;
    }

    void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    void setHigh() const {
        *info::port |= info::bitmask;
    }

    void setLow() const {
        *info::port &= ~info::bitmask;
    }

    bool isHigh() const {
        return (*info::pin & info::bitmask) != 0;
    }

    bool isLow() const {
        return (*info::pin & info::bitmask) == 0;
    }
};

template <typename pinInfo, typename extInterruptInfo>
class ExtInterruptPin: public Pin<pinInfo>, public ExtInterrupt<extInterruptInfo> {};

struct NoUsart {};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity, class Enable=void>
class UsartTxPin: public Pin<pinInfo> {
    typedef typename usart_t::fail error_wrong_usart_template_argument;
};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity>
class UsartTxPin<pinInfo, usart_t, writeFifoCapacity, typename std::enable_if<std::is_same<usart_t, NoUsart>::value>::type>: public Pin<pinInfo> {
    // If NoUsart is provided as usart_t, the pin will be defined without usart capability.
};

template <typename pinInfo, typename usart_t, uint8_t writeFifoCapacity>
class UsartTxPin<pinInfo, usart_t, writeFifoCapacity, typename std::enable_if<std::is_same<typename usart_t::usart_info_t, typename pinInfo::usart_info_t>::value>::type>:
    public Pin<pinInfo>, public UsartTx<typename usart_t::usart_info_t, writeFifoCapacity> {
    typedef UsartTxPin<pinInfo, usart_t, writeFifoCapacity> This;
    void onUSART_UDRE() {
        UsartTx<typename usart_t::usart_info_t, writeFifoCapacity>::onSendComplete();
    }
public:
    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_UDRE), onUSART_UDRE);
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity, class Enable=void>
class UsartRxPin: public Pin<pinInfo> {
    typedef typename usart_t::fail error_wrong_usart_template_argument;
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity>
class UsartRxPin<pinInfo, usart_t, readFifoCapacity, typename std::enable_if<std::is_same<usart_t, NoUsart>::value>::type>: public Pin<pinInfo> {
    // If NoUsart is provided as usart_t, the pin will be defined without usart capability.
};

template <typename pinInfo, typename usart_t, uint8_t readFifoCapacity>
class UsartRxPin<pinInfo, usart_t, readFifoCapacity, typename std::enable_if<std::is_same<typename usart_t::usart_info_t, typename pinInfo::usart_info_t>::value>::type>:
    public Pin<pinInfo>, public UsartRx<typename usart_t::usart_info_t, readFifoCapacity> {
    typedef UsartRxPin<pinInfo, usart_t, readFifoCapacity> This;
    void onUSART_RX() {
        UsartRx<typename usart_t::usart_info_t, readFifoCapacity>::onReceive();
    }
public:
    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_RX), onUSART_RX);
};

template <typename pinInfo>
class ADCOnlyPin {
    typedef pinInfo info_t;
};

struct NoTimer {} ;

template <typename pinInfo, typename timer_t, class Enable=void>
class PinOnComparatorA: public Pin<pinInfo> {
    typedef typename timer_t::fail error_wrong_timer_template_argument;
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorA<pinInfo, timer_t, typename std::enable_if<std::is_same<timer_t, NoTimer>::value>::type>: public Pin<pinInfo> {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorA<pinInfo, timer_t, typename std::enable_if<std::is_same<typename timer_t::timer_info_t, typename pinInfo::timer_info_t>::value>::type>: public Pin<pinInfo> {
    timer_t *t;
public:
    typedef typename timer_t::comparatorA_t comparator_t;
    inline PinOnComparatorA(timer_t &_timer): t(&_timer) {}
    inline comparator_t &timerComparator() const {
        return t->comparatorA();
    }
    inline timer_t &timer() const {
        return *t;
    }
};

template <typename pinInfo, typename timer_t, class Enable=void>
class PinOnComparatorB: public Pin<pinInfo> {
    typedef typename timer_t::error_wrong_timer_template_argument fail;
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorB<pinInfo, timer_t, typename std::enable_if<std::is_same<timer_t, NoTimer>::value>::type>: public Pin<pinInfo> {
    // If NoTimer is provided as timer_t, the pin will be defined without timer capability.
};

template <typename pinInfo, typename timer_t>
class PinOnComparatorB<pinInfo, timer_t, typename std::enable_if<std::is_same<typename timer_t::timer_info_t, typename pinInfo::timer_info_t>::value>::type>:
    public Pin<pinInfo>,
    public Time::Prescaled<typename timer_t::value_t, typename timer_t::prescaler_t, timer_t::prescaler> {
    timer_t *t;
public:
    typedef typename timer_t::comparatorB_t comparator_t;
    inline PinOnComparatorB(timer_t &_timer): t(&_timer) {}
    inline comparator_t &timerComparator() const {
        return t->comparatorB();
    }
    inline timer_t &timer() const {
        return *t;
    }

    void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    void setHigh() const {
        *pinInfo::port |= pinInfo::bitmask;
        if (timerComparator().isOutputConnected()) {
            // If one of the timer comparator modes is enabled, the pin's port bit actually doesnt do anything.
            // Instead, we change output mode. This only works for non-PWM modes, but during PWM it doesn't make
            // sense to call setHigh() anyways.

            // TODO move this hack so it only is present on PinOnComparatorA/B that are linked to a NonPWMComparator
            typedef typename comparator_t::comparator_info_t info;
            *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(Timer::NonPWMOutputMode::high_on_match) << info::output_mode_bitstart);
            *info::tccrb |= (1 << info::foc);
        }
    }

    void setLow() const {
        *pinInfo::port &= ~pinInfo::bitmask;
        if (timerComparator().isOutputConnected()) {
            // If one of the timer comparator modes is enabled, the pin's port bit actually doesnt do anything.
            // Instead, we change output mode. This only works for non-PWM modes, but during PWM it doesn't make
            // sense to call setHigh() anyways.

            // TODO move this hack so it only is present on PinOnComparatorA/B that are linked to a NonPWMComparator
            typedef typename comparator_t::comparator_info_t info;
            *info::tccra = (*info::tccra & ~(info::output_mode_bitmask)) | (static_cast<uint8_t>(Timer::NonPWMOutputMode::low_on_match) << info::output_mode_bitstart);
            *info::tccrb |= (1 << info::foc);
        }
    }
};

} // namespace AVR
} // namespace HAL


#endif /* HAL_ATMEL_PIN_HPP_ */
