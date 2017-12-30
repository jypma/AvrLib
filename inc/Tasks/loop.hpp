#pragma once

#include "gcc_type_traits.h"

using namespace Time;

namespace TasksImpl {
template<typename T>

struct has_loop_method
{
private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template<typename U> static auto test(int) -> decltype(std::declval<U>().loop(), yes());
    template<typename> static no test(...);
public:
    static constexpr bool value = std::is_same<decltype(test<T>(0)),yes>::value;
};


template <typename T>
void invokeIfHasLoop(T &t, std::true_type) {
    t.loop();
}

template <typename T>
void invokeIfHasLoop(T &t, std::false_type) {}

template <typename power_t, typename... types_t>
struct MultiLoop {
    static void setTaskStates(TaskState *dest) {
        // nothing for zero args
    }

    static void invokeLoop(types_t &... args) {
        // nothing for zero args
    }
};

template <typename power_t, typename head_t, typename... tail_t>
struct MultiLoop<power_t, head_t, tail_t...> {
    typedef MultiLoop<power_t, tail_t...> Tail;
    static constexpr uint8_t N = sizeof...(tail_t) + 1;

    static void setTaskStates(TaskState *dest, head_t &head, tail_t &... tail) {
        *dest = head.getTaskState();
        Tail::setTaskStates(dest + 1, tail...);
    }

    static void invokeLoop(head_t &head, tail_t &...tail) {
        invokeIfHasLoop(head, std::integral_constant<bool, has_loop_method<head_t>::value>());
        Tail::invokeLoop(tail...);
    }

    static void loop(power_t &power, head_t &head, tail_t &...tail) {
        TaskState states[N];
        setTaskStates(states, head, tail...);
        invokeLoop(head, tail...);
        power.sleepUntilTasks(states, N);
    }
};

}

/**
 * Automatically generates and invokes a "super-loop" function that does the following:
 * - Invokes getTaskState() on all of the arguments (expecting each to return a TaskState instance)
 * - Invokes loop() on all arguments, in sequence
 * - Sleeps for the lowest duration, and deepest allowed sleep state, according to the earlier-gathered task states.
 */
template <typename power_t, typename... types_t>
void loopTasks(power_t &power, types_t &... args) {
    TasksImpl::MultiLoop<power_t, types_t...>::loop(power, args...);
}
