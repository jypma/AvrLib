#pragma once

#include "HAL/Atmel/Power.hpp"

namespace Impl {

template <typename time_t>
class TaskState {
	bool idle;
	time_t tLeft;
	HAL::Atmel::SleepMode maxSleepMode;
public:
	/**
	 * Returns the deepest sleep mode this task can allow the system to go into, when the task
	 * is not idle.
	 */
	HAL::Atmel::SleepMode getMaxSleepMode() const { return maxSleepMode; }

	constexpr TaskState(bool i, time_t t, HAL::Atmel::SleepMode s): idle(i), tLeft(t), maxSleepMode(s) {}

	/**
	 * Returns whether this task is currently idle.
	 */
	bool isIdle() const { return idle; }

	/**
	 * Returns how much time the current (non-idle) task expects to need before being idle again.
	 * Any task defines this; for undefined untimes, the task should then implement and return
	 * a reasonable timeout.
	 */
	time_t timeLeft() const { return tLeft; }
};

}

/**
 * Returns a task state from the given Deadline instance, marking the task as idle
 * if the deadline isn't currently scheduled.
 */
template <typename deadline_t>
auto TaskState(deadline_t t, HAL::Atmel::SleepMode s) -> Impl::TaskState<decltype(t.timeLeft())>  {
	return { !t.isScheduled(), t.timeLeft(), s };
}
