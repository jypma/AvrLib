#ifndef EXTERNALINTERRUPT_HPP_
#define EXTERNALINTERRUPT_HPP_

#include <avr/common.h>
#include <avr/interrupt.h>

namespace HAL {
namespace Atmel {

template <typename info>
class ExtInterrupt {
public:
    typedef typename info::INT INT;

    /**
     * Disables raising any interrupts for this pin
     */
    void interruptOff() {
        info::off();
    }

    /**
     * Invokes an attached interrupt handler whenever the pin is low. Works in all sleep modes.
     * You should call interruptOff() from your handler, otherwise it might be
     * repeatedly invoked if the pin is still low when the interrupt handler returns.
     */
    void interruptOnLow() {
        info::on(0);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin changes value. Only works when
     * the I/O clock is running.
     */
    void interruptOnChange() {
        info::on(1);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from low to high. Only works when
     * the I/O clock is running.
     */
    void interruptOnRising() {
        info::on(2);
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from high to low. Only works when
     * the I/O clock is running.
     */
    void interruptOnFalling() {
        info::on(3);
    }
};

} // namespace Atmel
} // namespace HAL


#endif /* EXTERNALINTERRUPT_HPP_ */
