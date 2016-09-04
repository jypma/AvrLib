#include "HAL/Atmel/InterruptHandlers.hpp"

/**
 * Invokes an interrupt handler declared on the given target, e.g.
 *
 *     invoke<PinPD2::INT>(button);
 *
 */
template <typename int_t, typename target_t>
void invoke(target_t &target) {
    target_t::Handlers::template Handler<int_t>::invoke(target);
}
