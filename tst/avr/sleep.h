#ifndef SLEEP_H
#define SLEEP_H

#include <functional>

#define SLEEP_MODE_PWR_DOWN 0
#define SLEEP_MODE_STANDBY 1
#define SLEEP_MODE_IDLE 2

extern std::function<void()> onSleep_cpu;

inline void set_sleep_mode(uint8_t mode) {}
inline void sleep_enable() {}
inline void sleep_bod_disable()  {}
inline void sleep_cpu()  {
    if (onSleep_cpu != nullptr) {
        onSleep_cpu();
    }
}
inline void sleep_disable() {}

#endif
