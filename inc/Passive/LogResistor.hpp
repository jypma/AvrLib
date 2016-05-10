#ifndef PASSIVE_LOGRESISTOR_HPP_
#define PASSIVE_LOGRESISTOR_HPP_

#include <HAL/Atmel/InterruptVectors.hpp>
#include <Time/RealTimer.hpp>
#include <Time/Units.hpp>

namespace Passive {

namespace Impl {

using namespace Time;

/**
 * Measures a variable resistor (e.g. an LDR) onto a logarithmic scale, by making it charge a capacitor
 * and measuring charge time (rather than using the A/D converter directly). The measured time is then
 * logarithmically mapped into an uint8_t. It needs two pins to do this:
 *
 * [ pin_out ]-------[LDR]---+---[ C ]--------| GND
 *                           |
 *                       [ pin_in ]
 *
 * @param C       Size of the capacitor, in pF
 * @param R_min   Minimum resistance to measure (will map to 0 on output)
 * @param R_max   Maximum resistance to measure (will map to 255 on output)
 */
template <typename rt_t, typename pin_out_t, typename pin_in_t, uint64_t C, uint64_t R_min, uint64_t R_max>
class LogResistor {
    static_assert(R_min < R_max, "R_min must be less than R_max");
public:
    typedef LogResistor<rt_t, pin_out_t, pin_in_t, C, R_min, R_max> This;
    constexpr static uint32_t counts_min = toCountsOn<rt_t>((1_us).times<C * R_min / 1000000>());
    constexpr static uint32_t counts_max = toCountsOn<rt_t>((1_us).times<C * R_max / 1000000>());

    rt_t *rt;
    pin_out_t *pin_out;
    pin_in_t *pin_in;
    uint32_t startTime = 0;
    volatile uint32_t time = 0;
    volatile bool measuring = false;

    uint8_t fakelogValue() const {
        union {
            uint32_t intValue;
            uint8_t bytes[4];
        } v;
        v.intValue = time - counts_min;
        if (v.bytes[3] & 0b10000000) return (v.bytes[3] & (0b111 << 4) >> 4) + (8 * 29);
        if (v.bytes[3] & 0b01000000) return (v.bytes[3] & (0b111 << 3) >> 3) + (8 * 28);
        if (v.bytes[3] & 0b00100000) return (v.bytes[3] & (0b111 << 2) >> 2) + (8 * 27);
        if (v.bytes[3] & 0b00010000) return (v.bytes[3] & (0b111 << 1) >> 1) + (8 * 26);
        if (v.bytes[3] & 0b00001000) return (v.bytes[3] & (0b111 << 0) >> 0) + (8 * 25);
        if (v.bytes[3] & 0b00000100) return ((v.bytes[3] & 0b011 << 1) | (v.bytes[2] & 0b10000000 >> 7)) + (8 * 24);
        if (v.bytes[3] & 0b00000010) return ((v.bytes[3] & 0b001 << 2) | (v.bytes[2] & 0b11000000 >> 6)) + (8 * 23);
        if (v.bytes[3] & 0b00000001) return (v.bytes[2] & (0b111 << 5) >> 5) + (8 * 22);
        if (v.bytes[2] & 0b10000000) return (v.bytes[2] & (0b111 << 4) >> 4) + (8 * 21);
        if (v.bytes[2] & 0b01000000) return (v.bytes[2] & (0b111 << 3) >> 3) + (8 * 20);
        if (v.bytes[2] & 0b00100000) return (v.bytes[2] & (0b111 << 2) >> 2) + (8 * 19);
        if (v.bytes[2] & 0b00010000) return (v.bytes[2] & (0b111 << 1) >> 1) + (8 * 18);
        if (v.bytes[2] & 0b00001000) return (v.bytes[2] & (0b111 << 0) >> 0) + (8 * 17);
        if (v.bytes[2] & 0b00000100) return ((v.bytes[2] & 0b011 << 1) | (v.bytes[1] & 0b10000000 >> 7)) + (8 * 16);
        if (v.bytes[2] & 0b00000010) return ((v.bytes[2] & 0b001 << 2) | (v.bytes[1] & 0b11000000 >> 6)) + (8 * 15);
        if (v.bytes[2] & 0b00000001) return (v.bytes[1] & (0b111 << 5) >> 5) + (8 * 14);
        if (v.bytes[1] & 0b10000000) return (v.bytes[1] & (0b111 << 4) >> 4) + (8 * 13);
        if (v.bytes[1] & 0b01000000) return (v.bytes[1] & (0b111 << 3) >> 3) + (8 * 12);
        if (v.bytes[1] & 0b00100000) return (v.bytes[1] & (0b111 << 2) >> 2) + (8 * 11);
        if (v.bytes[1] & 0b00010000) return (v.bytes[1] & (0b111 << 1) >> 1) + (8 * 10);
        if (v.bytes[1] & 0b00001000) return (v.bytes[1] & (0b111 << 0) >> 0) + (8 * 9);
        if (v.bytes[1] & 0b00000100) return ((v.bytes[1] & 0b011 << 1) | (v.bytes[0] & 0b10000000 >> 7)) + (8 * 8);
        if (v.bytes[1] & 0b00000010) return ((v.bytes[1] & 0b001 << 2) | (v.bytes[0] & 0b11000000 >> 6)) + (8 * 7);
        if (v.bytes[1] & 0b00000001) return (v.bytes[0] & (0b111 << 5) >> 5) + (8 * 6);
        if (v.bytes[0] & 0b10000000) return (v.bytes[0] & (0b111 << 4) >> 4) + (8 * 5);
        if (v.bytes[0] & 0b01000000) return (v.bytes[0] & (0b111 << 3) >> 3) + (8 * 4);
        if (v.bytes[0] & 0b00100000) return (v.bytes[0] & (0b111 << 2) >> 2) + (8 * 3);
        if (v.bytes[0] & 0b00010000) return (v.bytes[0] & (0b111 << 1) >> 1) + (8 * 2);
        if (v.bytes[0] & 0b00001000) return (v.bytes[0] & (0b111 << 0) >> 0) + (8 * 1);
        return v.bytes[0] & 0b111;
    }

    void onPinChange() {
        pin_in->interruptOff();
        if (measuring) {
            measuring = false;
            time = rt->counts() - startTime;
            pin_out->setLow();
        }
    }
public:
    LogResistor(rt_t &_rt, pin_out_t &_pin_out, pin_in_t &_pin_in): rt(&_rt), pin_out(&_pin_out), pin_in(&_pin_in) {
        pin_out->configureAsOutput();
        pin_out->setLow();
        pin_in->configureAsInputWithoutPullup();
        pin_in->interruptOff();
    }

    void measure() {
        AtomicScope _;
        if (!measuring) {
            measuring = true;
            startTime = rt->counts();
            pin_out->setHigh();
            pin_in->interruptOnChange();
        }
    }

    bool isMeasuring() const {
        return measuring;
    }

    uint8_t getValue() const {
        if (time < counts_min) {
            return 0;
        } else if (time > counts_max) {
            return 255;
        } else {
            return fakelogValue();
        }
    }

    uint32_t getTime() const {
        return time;
    }

    INTERRUPT_HANDLER1(typename pin_in_t::INT, onPinChange);
};

}

template <uint64_t C, uint64_t R_min, uint64_t R_max, typename rt_t, typename pin_out_t, typename pin_in_t>
Impl::LogResistor<rt_t, pin_out_t, pin_in_t, C, R_min, R_max> LogResistor(rt_t &rt, pin_out_t &pin_out, pin_in_t &pin_in) {
    return Impl::LogResistor<rt_t, pin_out_t, pin_in_t, C, R_min, R_max>(rt, pin_out, pin_in);
}

}


#endif /* PASSIVE_LOGRESISTOR_HPP_ */
