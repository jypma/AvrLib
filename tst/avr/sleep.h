#ifndef SLEEP_H
#define SLEEP_H

#define SLEEP_MODE_PWR_DOWN 0

inline void set_sleep_mode(uint8_t mode) {}
inline void sleep_enable() {}
inline void sleep_bod_disable()  {}
inline void sleep_cpu()  {}
inline void sleep_disable() {}

#endif
