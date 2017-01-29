#pragma once

#include "avr/common.h"
#include "HAL/Atmel/Timer.hpp"
#include "Fifo.hpp"
#include "AtomicScope.hpp"
#include "Time/UnitLiterals.hpp"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Logging.hpp"

namespace Serial {

namespace Impl {

using namespace HAL::Atmel;
using namespace HAL::Atmel::InterruptHandlers;
using namespace Time;
using namespace Streams;

template <typename pin_t, uint32_t baudrate, uint8_t fifoSize>
class RS232Tx {
	typedef Logging::Log<Loggers::RS232Tx> log;
public:
	typedef typename pin_t::comparator_t::value_t count_t;
	typedef RS232Tx<pin_t, baudrate, fifoSize> This;

	static constexpr count_t bitLength = dividedBy<baudrate>((1_s).toCountsOn<typename pin_t::comparator_t>()).getValue();
	static_assert(bitLength > 5, "Bit length is too low. Decrease baudrate, or decrease timer prescaler.");
	static const count_t bitLengths[5];

	pin_t* const pin;
	Fifo<fifoSize> fifo;

	bool transmitting = false;
	count_t lengths[10] = {}; // worst case start bit + 8 up/down bits + stop bit
	uint8_t max = 0;
	uint8_t pos = 0;
	count_t startValue = 0;

	inline __attribute__((always_inline)) void nextByte(uint8_t byte) {
    	log::debug(F("next"));
		// ? calculate initial lengths as int8_t, not count_t
		// ? don't calculate lengths[], calculate targets[] instead

		max = 0;
		bool high = false;  // is the output currently high?
		lengths[max] = 1;     // we're low during the start bit
		for (uint8_t bit = 0; bit < 3; bit++) {
			// bits 0..2 can't produce a length > 4
			//std::cout << "bit " << int(bit) << ": " << ((byte & 1) != 0) << "/" << high << std::endl;
			if (((byte & 1) != 0) != high) {
				high = !high;
				max++;
				lengths[max] = 1;
			} else {
				lengths[max]++;
			}
			byte >>= 1;
		}
		for (uint8_t bit = 3; bit < 7; bit++) {
			// bits 3..6 need a length check
			//std::cout << "bit " << int(bit) << ": " << ((byte & 1) != 0) << "/" << high << std::endl;
			if (((byte & 1) != 0) != high) {
				high = !high;
				max++;
				lengths[max] = 1;
			} else {
				if (lengths[max] < 4) {
					lengths[max]++;
				} else {
					max++;
					lengths[max] = 0;
					max++;
					lengths[max] = 1;
				}
			}
			byte >>= 1;
		}
		// last data bit
		//std::cout << "bit 7: " << ((byte & 1) != 0) << "/" << high << std::endl;
		if (((byte & 1) != 0) != high) {
			high = !high;
			max++;
			lengths[max] = 1;
		} else {
			if (lengths[max] < 4) {
				lengths[max]++;
			} else {
				max++;
				lengths[max] = 0;
				max++;
				lengths[max] = 1;
			}
		}
		// stop bit (must be high)
		if (byte & 1) {
			// last data bit was also high -> lengthen
			if (lengths[max] < 4) {
				lengths[max]++;
			} else {
				max++;
				lengths[max] = 0;
				max++;
				lengths[max] = 1;
			}
		} else {
			// last data bit was low -> new transition
			max++;
			lengths[max] = 1;
		}
		for (uint8_t i = 0; i <= max; i++) {
			//std::cout << int(i) << ": " << int(lengths[i]) << " -> " << int(bitLengths[lengths[i]]) << std::endl;
			lengths[i] = bitLengths[lengths[i]];
		}
		pos = 0;

		AtomicScope _;
		startValue = pin->timerComparator().getValue();
		//log::debug('s', dec(startValue), 'l', dec(lengths[pos]));
		//std::cout << "start: " << int(startValue) << std::endl;
		//std::cout << "length: " << int(lengths[pos]) << std::endl;
		pin->timerComparator().setTarget(startValue + lengths[pos]);

		// We make the timer do the transition. Assuming high right now.
		pin->timerComparator().setOutput(NonPWMOutputMode::low_on_match);
		pin->timerComparator().applyOutput();
		pin->setLow(); // start bit


		if ((max > pos) && (lengths[pos+1] == 0)) {
			// first pulse is a long one
			pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
		} else {
			//pin->timerComparator().setOutput(NonPWMOutputMode::low_on_match); // this causes a glitch when OC0A is still high
			//pin->timerComparator().applyOutput();
			pin->timerComparator().setOutput(NonPWMOutputMode::toggle_on_match);
		}
		pin->timerComparator().interruptOn();
	}

    inline __attribute__((always_inline)) void onComparator() {
    	//std::cout << "1. pos: " << int(pos) << " max: " << int(max) << std::endl;
    	log::timeStart();
#ifdef SAFE
    	if (!transmitting) {
        	log::debug(F("X1"));
    		pin->timerComparator().interruptOff();
			pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
    		pin->setHigh();
    		log::timeEnd();
    		return;
    	}
#endif

    	pos++;
    	if (pos > max) {
    		uint8_t byte;
    		if (fifo.fastread(byte)) {
    			nextByte(byte);
    		} else {
            	log::debug(F("done."));
        		transmitting = false;
        		pin->timerComparator().interruptOff();
    			pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
        		pin->setHigh();
    		}
    		log::timeEnd();
			return;
    	}

    	if (lengths[pos] == 0) {
    		// we're entering the second half of a long pulse. Simply toggle on the next one.
    		pos++;
#ifdef SAFE
    		if (pos > max) {
    			// this shouldn't happen
        		transmitting = false;
        		pin->timerComparator().interruptOff();
    			pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
        		pin->setHigh();
        		log::timeEnd();
        		return;
    		}
#endif
    	} else {
    		// the timer has just toggled the real output, but we copy its state since we might disconnect later.
        	pin->setHigh(!pin->isOutputHigh());
    	}

    	pin->timerComparator().setTarget(pin->timerComparator().getTarget() + lengths[pos]);
    	//std::cout << "2. pos: " << int(pos) << " max: " << int(max) << std::endl;
		if (pos < max && lengths[pos+1] == 0) {
			// next pulse is a long one
			pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
		} else {
			if (pos < max) {
				// normal bit
				if (pin->timerComparator().getOutput() == NonPWMOutputMode::disconnected) {
					// copy current output state
					pin->timerComparator().setOutput(pin->isOutputHigh() ? NonPWMOutputMode::high_on_match : NonPWMOutputMode::low_on_match);
					pin->timerComparator().applyOutput();
				}
				pin->timerComparator().setOutput(NonPWMOutputMode::toggle_on_match);
			} else {
	        	// Attempt to force OC0A output high, so it latches high on the next byte.
				pin->timerComparator().setOutput(NonPWMOutputMode::high_on_match);
				pin->timerComparator().applyOutput();

				// stop bit -> keep high after we're done. Pin state should be good to go.
				pin->setHigh();
				pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
			}
		}
		log::timeEnd();
    }

	void transmit() {
		bool shouldStart;
		{
			AtomicScope _;
			shouldStart = !transmitting;
			transmitting = true;
		}

		if (shouldStart) {
			uint8_t b;
			fifo.read(&b);
			nextByte(b);
		}
	}
public:
    typedef On<This, typename pin_t::comparator_t::INT, &This::onComparator> Handlers;

	RS232Tx(pin_t &p): pin(&p) {
		pin->timerComparator().interruptOff();
		pin->timerComparator().setOutput(NonPWMOutputMode::high_on_match);
		pin->timerComparator().applyOutput();
		pin->configureAsOutputHigh();
		pin->timerComparator().setOutput(NonPWMOutputMode::disconnected);
	}

    template <typename... types>
    bool write(types... args) {
    	bool ok = fifo.write(args...);
    	if (ok && fifo.hasContent()) {
    		transmit();
    	}
    	return ok;
    }

    uint8_t getSize() {
    	return fifo.getSize();
    }
};

template <typename pin_t, uint32_t baudrate, uint8_t fifoSize>
const typename RS232Tx<pin_t,baudrate,fifoSize>::count_t RS232Tx<pin_t,baudrate,fifoSize>::bitLengths[5] = {
	0,
	RS232Tx<pin_t,baudrate,fifoSize>::bitLength,
	RS232Tx<pin_t,baudrate,fifoSize>::bitLength * 2,
	RS232Tx<pin_t,baudrate,fifoSize>::bitLength * 3,
	RS232Tx<pin_t,baudrate,fifoSize>::bitLength * 4
};

}

template <typename pin_t, uint32_t baudrate = 9600, uint8_t fifoSize = 32>
Impl::RS232Tx<pin_t, baudrate, fifoSize> RS232Tx(pin_t &pin) {
	return { pin };
}

}


