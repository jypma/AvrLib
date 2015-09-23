#ifndef HAL_ATMEL_PIN_HPP_
#define HAL_ATMEL_PIN_HPP_

#include <HAL/Atmel/Usart.hpp>

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
    UsartTxPin(const Usart<typename usart_t::usart_info_t> &usart) {}
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
    UsartRxPin(const Usart<typename usart_t::usart_info_t> &usart) {}
    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_RX), onUSART_RX);
};

} // namespace AVR
} // namespace HAL


#endif /* HAL_ATMEL_PIN_HPP_ */
