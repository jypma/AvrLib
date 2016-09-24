#ifndef DHT_DHT_HPP_
#define DHT_DHT_HPP_

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Serial/PulseCounter.hpp"
#include "Logging.hpp"
#include "Streams/Protocol.hpp"
#include "Time/RealTimer.hpp"
#include "AtomicScope.hpp"

namespace DHT {

using namespace Serial;
using namespace Time;
using namespace Streams;
using namespace HAL::Atmel::InterruptHandlers;

enum class DHTState: uint8_t {
    OFF, BOOTING, IDLE, SIGNALING, SYNC_LOW, SYNC_HIGH, RECEIVING_LOW, RECEIVING_HIGH
};

namespace Impl {

/** Abstract base class for all DHT-based temperature & humidity sensors */
template <typename datapin_t, typename powerpin_t, typename comparator_t, typename rt_t>
class DHT {
    typedef DHT<datapin_t,powerpin_t,comparator_t,rt_t> This;
    typedef Logging::Log<Loggers::DHT11> log;

    datapin_t *pin;
    powerpin_t *power;
    DHTState state;
    PulseCounter<comparator_t, datapin_t, 250> counter;
    VariableDeadline<rt_t> timeout;
    uint8_t bit = 7;
    uint8_t pos = 0;
    uint8_t data[4] = { 0, 0, 0, 0 };
    bool seenHigh = false;
    uint8_t lastFailure = 0;

public:
    void powerOff() {
    	AtomicScope _;
        power->setLow();
        state = DHTState::OFF;
    }

    void powerOn() {
    	AtomicScope _;
        if (state == DHTState::OFF) {
            log::debug(F("Booting"));
            pin->configureAsInputWithPullup();
            power->setHigh();
            timeout.schedule(1_s);
            state = DHTState::BOOTING;
        }
    }

    void measure() {
    	AtomicScope _;
        if (state == DHTState::OFF) {
            powerOn();
        } else if (state == DHTState::IDLE || state == DHTState::BOOTING || state == DHTState::SIGNALING) {
            log::debug(F("Starting measurement"));
            pin->configureAsOutput();
            pin->setLow();
            timeout.schedule(18_ms);
            state = DHTState::SIGNALING;
        } else {
            log::debug(F("Still booting, or measurement already in progress."));
        }
    }

protected:
    uint8_t getData(uint8_t idx) const {
        return data[idx];
    }
private:
    void reset(uint8_t failure) {
        log::debug(F("Resetting, err="), dec(failure));
        lastFailure = failure;
        pin->configureAsInputWithPullup();
        state = DHTState::IDLE;
        bit = 7;
        pos = 0;
        seenHigh = false;
        counter.pause();
    }

    void receive(bool value) {
        if (value) {
            data[pos] |= (1 << bit);
        } else {
            data[pos] &= ~(1 << bit);
        }
        if (bit > 0) {
            bit--;
        } else {
            bit = 7;
            log::debug(F("in "), dec(data[pos]));
            pos++;
        }
        if (pos >= 5) {
            reset(0);
        } else {
            state = DHTState::RECEIVING_LOW;
        }
    }

    void booting() {
        if (timeout.isNow()) {
            measure();
        }
    }

    void signaling() {
        if (timeout.isNow()) {
            log::debug(F("Switching to input"));
            pin->configureAsInputWithPullup();
            counter.resume();
            state = DHTState::SYNC_LOW;
        }
    }

    template <typename f_t>
    void expectPulse(f_t f) {
        counter.on([this, f] (auto pulse) {
            log::debug(dec(uint8_t(state)), ':', as<Pulse::TEXT>(&pulse));
            if (pulse.isEmpty()) {
                this->reset(uint8_t(state));
            } else {
                f(pulse);
            }
        });
    }

    void sync_low() {
        expectPulse([this] (auto pulse) {
            if (pulse.isLow() && pulse > 60_us && pulse < 120_us) {
                state = DHTState::SYNC_HIGH;
            }
        });
    }

    void sync_high() {
        expectPulse([this] (auto pulse) {
            if (pulse.isHigh() && pulse > 60_us && pulse < 120_us) {
                state = DHTState::RECEIVING_LOW;
            }
        });
    }

    void receiving_low() {
        expectPulse([this] (auto pulse) {
            if (pulse.isLow()) {
                if (pulse > 30_us && pulse < 80_us) {
                    state = DHTState::RECEIVING_HIGH;
                } else {
                    this->reset(pulse.getDuration());
                }
            } else {
                this->reset(43);
            }
        });
    }

    void receiving_high() {
        expectPulse([this] (auto pulse) {
            if (pulse.isHigh()) {
                if (pulse < 50_us) {
                    this->receive(0);
                } else {
                    this->receive(1);
                }
            } else {
                this->reset(44);
            }
        });
    }

public:
    typedef Delegate<This, decltype(counter), &This::counter> Handlers;

    DHT(datapin_t &_pin, powerpin_t &_power, comparator_t &_comparator, rt_t &_rt):
        pin(&_pin),
        power(&_power),
        state(DHTState::OFF),
        counter(_comparator, _pin),
        timeout(deadline(_rt)) {
        power->configureAsOutput();
        powerOn();
    }

    void loop() {
        switch(state) {
        case DHTState::OFF: break;
        case DHTState::IDLE: break;
        case DHTState::BOOTING: booting(); break;
        case DHTState::SIGNALING: signaling(); break;
        case DHTState::SYNC_LOW: sync_low(); break;
        case DHTState::SYNC_HIGH: sync_high(); break;
        case DHTState::RECEIVING_LOW: receiving_low(); break;
        case DHTState::RECEIVING_HIGH: receiving_high(); break;
        }
    }

    DHTState getState() const {
        return state;
    }

    bool isIdle() const {
        return state == DHTState::IDLE;
    }

    bool isMeasuring() const {
    	return !isIdle();
    }

    /** Returns any failure code that occurred during the most recent measurement, or 0 for no failure. */
    uint8_t getLastFailure() const {
        return lastFailure;
    }
};

} // namespace Impl

} // namespace DHT


#endif /* DHT_DHT_HPP_ */
