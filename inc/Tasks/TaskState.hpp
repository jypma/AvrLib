#pragma once

#include "HAL/Atmel/SleepMode.hpp"
#include "Time/RealTimer.hpp"
#include "Time/Units.hpp"
#include "Time/UnitLiterals.hpp"
#include "Option.hpp"

using namespace Time;

class TaskState {
	Option<Milliseconds> tLeft;
	HAL::Atmel::SleepMode maxSleepMode;

	template <typename rt_t, typename value>
	static constexpr Option<Milliseconds> deadlineMillis(Time::Deadline<rt_t, value> d) {
		if (d.isScheduled()) {
			return some(toMillisOn<rt_t>(d.timeLeft()));
		} else {
			return none();
		}
	}

	template <typename rt_t>
	static constexpr Option<Milliseconds> deadlineMillis(Time::VariableDeadline<rt_t> d) {
		if (d.isScheduled()) {
			return some(toMillisOn<rt_t>(d.timeLeft()));
		} else {
			return none();
		}
	}

public:
	/**
	 * Returns the deepest sleep mode this task can allow the system to go into, when the task
	 * is not idle.
	 */
	HAL::Atmel::SleepMode getMaxSleepMode() const { return maxSleepMode; }

	constexpr TaskState(Option<Milliseconds> t, HAL::Atmel::SleepMode s): tLeft(t), maxSleepMode(s) {}

	/**
	 * Returns a TaskState from the given Deadline instance, marking the task as idle
	 * if the deadline isn't currently scheduled.
	 */
	template <typename rt_t, typename value>
	constexpr TaskState(Time::Deadline<rt_t,value> d, HAL::Atmel::SleepMode s): tLeft(deadlineMillis(d)), maxSleepMode(s) {}

	/**
	 * Returns a TaskState from the given VariableDeadline instance, marking the task as idle
	 * if the deadline isn't currently scheduled.
	 */
	template <typename rt_t>
	constexpr TaskState(Time::VariableDeadline<rt_t> d, HAL::Atmel::SleepMode s): tLeft(deadlineMillis(d)), maxSleepMode(s) {}

	/**
	 * Returns a TaskState indicating a non-idle task, which will run for [time] allowing sleep mode [s].
	 * FIXME put in a check that it's actually a time unit
	 */
	template <typename time_t>
	static constexpr TaskState busy(time_t time, HAL::Atmel::SleepMode s) {
		const Milliseconds t = time.toMillis();
		return { some(t), s };
	}

	/**
	 * Returns a TaskState indicating an interrupt-driven task, i.e. a task that is always
	 * allowing sleep of the given mode, for an indeterminate amount of time.
	 */
	static constexpr TaskState busy(HAL::Atmel::SleepMode mode) {
		return busy(60_s, mode);
	}

	/**
	 * A TaskState indicating an idle task, that allows for the system to go to full shutdown.
	 */
	static constexpr TaskState idle() {
		return { none(), HAL::Atmel::SleepMode::POWER_DOWN };
	}

	/**
	 * Returns whether this task is currently idle.
	 */
	constexpr bool isIdle() const { return tLeft.isEmpty(); }

	/**
	 * For non-idle tasks, returns the amount of time until the task will reach its next deadline or timeout.
	 * This method must not be called for idle tasks.
	 * Any task defines this; for unknown run-times, the task should then implement and return
	 * a reasonable timeout.
	 */
	constexpr Milliseconds timeLeft() const { return tLeft.get(); }
};
