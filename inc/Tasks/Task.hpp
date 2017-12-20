#pragma once

class Task {
public:
    // By default, tasks return an idle task state. Subclasses can redefine this method to return an actual task state.
    static constexpr TaskState getTaskState() {
        return TaskState::idle();
    }

    // By default, a task's loop function does nothing. Interrupt-only tasks can leave it like that.
    static void loop() {}
};

