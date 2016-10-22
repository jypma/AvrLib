#ifndef HAL_ATMEL_PIN_HPP_
#define HAL_ATMEL_PIN_HPP_

#include <HAL/Atmel/Usart.hpp>
#include "HAL/Atmel/ExternalInterrupt.hpp"
#include "HAL/Atmel/PinChangeInterrupt.hpp"
#include "HAL/Atmel/Timer.hpp"
#include "HAL/attributes.hpp"

namespace HAL {
namespace Atmel {

using namespace InterruptHandlers;

template <typename info>
class Pin {
public:
    constexpr Pin() {}

    typedef info info_t;

    INLINE void configureAsOutput() const {
        info::configureAsGPIO();
        info::DDR.set();
    }

    INLINE void configureAsInputWithoutPullup() const {
        info::configureAsGPIO();
        info::DDR.clear();
        info::PORT.clear();
    }
    INLINE void configureAsInputWithPullup() const {
    	info::DDR.clear();
    	info::PORT.set();
    }

    INLINE void setHigh (bool on) const {
        if (on) {
            setHigh();
        } else {
            setLow();
        }
    }

    INLINE void setHigh() const {
        info::PORT.set();
    }

    INLINE void setLow() const {
        info::PORT.clear();
    }

    INLINE bool isOutputHigh() const {
        return info::PORT.isSet();
    }

    INLINE bool isOutputLow() const {
        return info::PORT.isCleared();
    }

    INLINE bool isHigh() const {
        return info::PIN.isSet();
    }

    INLINE bool isLow() const {
        return info::PIN.isCleared();
    }

    INLINE void configureAsOutputLow() const {
        setLow();
        configureAsOutput();
    }

    INLINE void configureAsOutputHigh() const {
        setHigh();
        configureAsOutput();
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
    public Pin<pinInfo>, public UsartTx<typename usart_t::usart_info_t, writeFifoCapacity> {};

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
    public Pin<pinInfo>, public UsartRx<typename usart_t::usart_info_t, readFifoCapacity> {};

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
};

template <typename base, typename info>
class WithPinChange: public base, public PinChangeInterrupt<typename info::pcintInfo, (1 << info::pcintBit)> {
public:
	using base::base;
};

template <typename base, typename info>
class WithPinChangeOnChange: public base, public PinChangeInterruptOnChange<typename info::pcintInfo, (1 << info::pcintBit)> {
public:
	using base::base;
};

template <typename base, typename info>
class WithPinChangeOption: public base {
public:
	using base::base;

    static WithPinChange<base, info> withInterrupt() {
        return WithPinChange<base, info>();
    }
    static WithPinChangeOnChange<base, info> withInterruptOnChange() {
    	return WithPinChangeOnChange<base, info>();
    }
};

template <typename info>
class PinWithPinChange: public WithPinChange<Pin<info>, info> {
	typedef WithPinChange<Pin<info>, info> Super;
public:
	using Super::Super;
};

template <typename info>
class PinWithPinChangeOption: public WithPinChangeOption<Pin<info>, info> {
	typedef WithPinChangeOption<Pin<info>, info> Super;
public:
	using Super::Super;
};

} // namespace AVR
} // namespace HAL


#endif /* HAL_ATMEL_PIN_HPP_ */
