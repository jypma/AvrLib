#include "HAL/Atmel/InterruptHandlers.hpp"

template <typename int_t, typename target_t>
void invoke(target_t &target) {
    target_t::Handlers::template Handler<int_t>::invoke(target);
}
