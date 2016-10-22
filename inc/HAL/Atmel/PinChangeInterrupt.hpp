#ifndef HAL_ATMEL_PINCHANGEINTERRUPT_HPP_
#define HAL_ATMEL_PINCHANGEINTERRUPT_HPP_

#include "AtomicScope.hpp"
#include "Logging.hpp"

namespace HAL {
namespace Atmel {

template <typename pcintInfo, uint8_t bitmask>
class PinChangeSupport {
	typedef Logging::Log<Loggers::PinChangeInterrupt> log;

    static uint8_t last;
    static uint8_t rising;
    static uint8_t directional;

    static bool shouldInvoke(uint8_t now) {
        uint8_t changed = now ^ last;
        if (!(changed & bitmask)) return false;
        if (!(directional & bitmask)) return true;
        return (now & bitmask) == (rising & bitmask);
    }

    static void enablePCINT() {
        AtomicScope _;


        if (pcintInfo::PCIE.isCleared()) {
            last = pcintInfo::PIN.get();
            pcintInfo::PCIE.set();
        }
    }

    static inline __attribute__((always_inline)) void disablePCINTIfNeeded() {
        AtomicScope _;

        if (pcintInfo::PCMSK.get() == 0) { // no more handlers are registered
        	pcintInfo::PCIE.clear();
        }
    }

public:
    //static volatile uint8_t ints;

    template <typename body_t>
    static inline __attribute__((always_inline)) void wrap(body_t body) {
    	log::timeStart();
    	//ints++;
        uint8_t now = pcintInfo::PIN.val();
        if (shouldInvoke(now)) {
            body();
        }
        last = now;
        log::timeEnd();
    }

    static void interruptOnChange() {
        directional &= ~bitmask;
        enablePCINT();
        pcintInfo::PCMSK.val() |= bitmask;
    }

    static void interruptOnRising() {
        rising |= bitmask;
        directional |= bitmask;
        enablePCINT();
        pcintInfo::PCMSK.val() |= bitmask;
    }

    static void interruptOnFalling() {
        rising &= ~bitmask;
        directional |= bitmask;
        enablePCINT();
        pcintInfo::PCMSK.val() |= bitmask;
    }

    __attribute__((always_inline)) inline static void interruptOff() {
        pcintInfo::PCMSK.val() &= ~bitmask;
        disablePCINTIfNeeded();
    }
};

template <typename pcintInfo, uint8_t bitmask> uint8_t PinChangeSupport<pcintInfo,bitmask>::last = 0;
template <typename pcintInfo, uint8_t bitmask> uint8_t PinChangeSupport<pcintInfo,bitmask>::rising = 0;
template <typename pcintInfo, uint8_t bitmask> uint8_t PinChangeSupport<pcintInfo,bitmask>::directional = 0;

template <typename pcintInfo, uint8_t bitmask>
struct PinChangeVector {
    typedef typename pcintInfo::PCINT INT;
    typedef PinChangeSupport<pcintInfo, bitmask> support;

    template <typename body_t>
    static __attribute__((always_inline)) inline void wrap(body_t body) {
        support::wrap(body);
    }
};

template <typename pcintInfo, uint8_t bitmask>
class PinChangeInterrupt {
public:
    typedef PinChangeVector<pcintInfo, bitmask> INT;

    /**
     * Invokes an attached interrupt handler whenever the pin changes value.
     */
    void interruptOnChange() {
        INT::support::interruptOnChange();
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from low to high.
     */
    void interruptOnRising() {
        INT::support::interruptOnRising();
    }

    /**
     * Invokes an attached interrupt handler whenever the pin goes from high to low.
     */
    void interruptOnFalling() {
        INT::support::interruptOnFalling();
    }

    /**
     * TODO have this call the handler repeatedly _while_ the pin is low. Currently,
     * we just call interruptOnFalling().
     */
    void interruptOnLow() {
        interruptOnFalling();
    }

    __attribute__((always_inline)) inline void interruptOff() {
        INT::support::interruptOff();
    }
};

template <typename pcintInfo, uint8_t bitmask>
class PinChangeVectorOnChange {
	typedef Logging::Log<Loggers::PinChangeInterrupt> log;

    static uint8_t last;

    static void enablePCINT() {
        AtomicScope _;

        if (pcintInfo::PCIE.isCleared()) {
            last = pcintInfo::PIN.get();
            pcintInfo::PCIE.set();
        }
    }

    static inline __attribute__((always_inline)) void disablePCINTIfNeeded() {
        AtomicScope _;

        if (pcintInfo::PCMSK == 0) { // no more handlers are registered
            pcintInfo::PCIE.clear();
        }
    }
public:
    // The actual interrupt this listens on
	typedef typename pcintInfo::PCINT INT;

    __attribute__((always_inline)) inline static void interruptOnChange() {
        enablePCINT();
        pcintInfo::PCMSK.val() |= bitmask;
    }

    __attribute__((always_inline)) inline static void interruptOff() {
    	pcintInfo::PCMSK.val() &= ~bitmask;
		disablePCINTIfNeeded();
    }

    template <typename body_t>
    static __attribute__((always_inline)) inline void wrap(body_t body) {
    	log::timeStart();
        uint8_t now = pcintInfo::PIN.val();
        uint8_t changed = now ^ last;
        if (changed & bitmask) {
            body();
        }
        last = now;
        log::timeEnd();
    }
};

template <typename pcintInfo, uint8_t bitmask> uint8_t PinChangeVectorOnChange<pcintInfo, bitmask>::last = 0;

template <typename pcintInfo, uint8_t bitmask>
class PinChangeInterruptOnChange {
public:
    typedef PinChangeVectorOnChange<pcintInfo, bitmask> INT;

    /**
     * Invokes an attached interrupt handler whenever the pin changes value.
     */
    void interruptOnChange() {
        INT::interruptOnChange();
    }

    __attribute__((always_inline)) inline void interruptOff() {
        INT::interruptOff();
    }
};

}
}


#endif /* HAL_ATMEL_PINCHANGEINTERRUPT_HPP_ */
