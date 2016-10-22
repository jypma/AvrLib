#pragma once

#include <stdint.h>

namespace HAL {
namespace Atmel {

enum class SleepMode: uint8_t {
    /** Lowest power mode. */
    POWER_DOWN = 2,
    /**
     * Like POWER_DOWN, but leaves the oscillator running so resuming is (a lot) faster. Use this if you're expecting
     * serial or SPI data to come in via interrupts that you need to quickly read in.
     */
    STANDBY = 1,
    /**
     * Least power savings, basically only halts the CPU and Flash. But this is the only sleep mode
     * where PWM keeps running.
     */
    IDLE = 0
};

}
}


