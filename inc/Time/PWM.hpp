#pragma once

namespace Time {

namespace Impl {

template <typename timer_t>
class PWM {
	timer_t* const timer;
	typename timer_t::value_t pulseWidth = 0;
	typename timer_t::value_t nextPulseWidth = 0;
	typename timer_t::value_t late = 0;
	typename timer_t::value_t nextLate = 0;
	bool lastHigh = false;
public:
	PWM(timer_t &t): timer(&t) {}

	void setPulseWidth(typename timer_t::value_t w) {
		pulseWidth = w;
		nextPulseWidth = w - late;
	}

	bool isHigh() {
		const auto value = timer->getValue();
		if (lastHigh) {
			if (value < nextPulseWidth) {
				// still high
				return true;
			} else {
				// transition to low
				nextLate = value - nextPulseWidth;
				if (nextLate >= pulseWidth) {
					nextLate = pulseWidth - 1;
				}
				lastHigh = false;
				return false;
			}
		} else {
			if (value < nextPulseWidth) {
				// transition to high
				late = nextLate;
				nextPulseWidth = pulseWidth - late;
				lastHigh = true;
				return true;
			} else {
				// still low
				return false;
			}
		}
	}
};

}

template <typename timer_t>
Impl::PWM<timer_t> PWM(timer_t &t) {
	return { t };
}

}


