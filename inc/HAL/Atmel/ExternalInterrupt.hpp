#ifndef EXTERNALINTERRUPT_HPP_
#define EXTERNALINTERRUPT_HPP_

#include "HAL/attributes.hpp"

namespace HAL {
namespace Atmel {

template <typename info>
class ExtInterrupt {
	template <typename bits_t>
	INLINE void interruptOn(bits_t bits) {
		info::INTF.set();
		info::EICR.apply(bits);
		info::INT_r.set();
	}
public:
    typedef typename info::INT INT;

    /**
     * Disables raising any interrupts for this pin
     */
    INLINE void interruptOff() {
        info::INT_r.clear();
    }

    /**
     * Invokes an attached interrupt handler whenever the pin is low. Works in all sleep modes.
     * You should call interruptOff() from your handler, otherwise it might be
     * repeatedly invoked if the pin is still low when the interrupt handler returns.
     */
    INLINE void interruptOnLow() {
        interruptOn(~info::ISC0 & ~info::ISC1);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin changes value. Only works when
     * the I/O clock is running.
     */
    INLINE void interruptOnChange() {
    	interruptOn(info::ISC0 & ~info::ISC1);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from low to high. Only works when
     * the I/O clock is running.
     */
    INLINE void interruptOnRising() {
    	interruptOn(info::ISC0 | info::ISC1);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from high to low. Only works when
     * the I/O clock is running.
     */
    INLINE void interruptOnFalling() {
    	interruptOn(~info::ISC0 | info::ISC1);
    }
};

} // namespace Atmel
} // namespace HAL


#endif /* EXTERNALINTERRUPT_HPP_ */
