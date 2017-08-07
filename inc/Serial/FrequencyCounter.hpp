#pragma once

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Logging.hpp"
#include "Time/Units.hpp"
#include "Option.hpp"
#include <string.h>

namespace Serial {

using namespace Streams;
using namespace Time;
using namespace HAL::Atmel::InterruptHandlers;

template <typename pin_t, typename rt_t>
class FrequencyCounter {
    typedef FrequencyCounter<pin_t, rt_t> This;
    typedef Logging::Log<Loggers::FrequencyCounter> log;

    pin_t *const pin;
    rt_t *const rt;

    static constexpr uint8_t N = 4;
    Counts times[N] = { 0, 0, 0, 0 };
    uint8_t pos = 0;

    void onPinChanged() {
        times[pos] = rt->counts();
        pos = (pos + 1) % N;
    }

public:
    typedef On<This, typename pin_t::INT, &This::onPinChanged> Handlers;

    FrequencyCounter(pin_t &_pin, rt_t &_rt): pin(&_pin), rt(&_rt) {}

    Option<uint16_t> getFrequency() {
        Counts times[N] = {0, 0, 0, 0};
        uint8_t pos;
        {
            AtomicScope _;
            memcpy(&times, &this->times, sizeof(times));
            pos = this->pos;
        }

        Counts total = 0;
        for (uint8_t i = 0; i < N - 1; i++) {
            const uint8_t p = (pos + i) % N; // times[pos] is the oldest time
            const Counts l = times[(p + 1) % N] - times[p];
            if (l == 0) {
                log::debug(F("two pin changes at the same timestamp, at "), dec(i));
                // no value recorded yet?
                return none();
            }
            if (i > 0) {
                const Counts last = total / i;
                const Counts diff = l > last ? l - last : last - l;
                if (diff != 0 && ((last / diff) < 5)) {
                    log::debug(F("too different, last:"), dec(last.getValue()), F(", diff:"), dec(diff.getValue()));
                    // too different (more than 1/5th) from previous value
                    return none();
                }
            }
            total += l;
        }
        const Counts len = total / (N - 1);
        return uint32_t(1000000) / toMicrosOn<rt_t>(len).getValue();
    }
};

}


