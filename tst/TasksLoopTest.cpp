#include <gtest/gtest.h>
#include "Mocks.hpp"
#include "Time/RealTimer.hpp"
#include "Time/UnitLiterals.hpp"
#include "Tasks/TaskState.hpp"
#include "Tasks/loop.hpp"
#include "Tasks/Task.hpp"

namespace TasksLoopTest {

using namespace Mocks;

struct MockPower {
    bool haveSlept = false;

    bool sleepUntilTasks(TaskState *states, uint8_t N) {
        haveSlept = true;
        return false;
    }
};

struct MockTask {
    TaskState taskState;
    bool haveLooped = false;

    TaskState getTaskState() {
        return taskState;
    }

    void loop() {
        haveLooped = true;
    }
};

// An interrupt-only driven task
struct TaskWithNoLoop {
    TaskState taskState;

    TaskState getTaskState() {
        return taskState;
    }
};

// Just here to check that anything extending Task will compile
struct BasicTask: public Task {

};

struct MockHandler {
    bool invoked = false;

    void invoke() {
        invoked = true;
    }
};

TEST(TasksLoopTest, loop_should_invoke_all_tasks_and_then_sleep) {
    auto rt = MockRealTimer();
    auto power = MockPower();
    MockHandler perHandler = {};

    MockTask idleTask = { TaskState::idle() };
    TaskWithNoLoop sleepStandbyTask = { TaskState::busy(1_s, SleepMode::STANDBY) };
    BasicTask basicTask = {};

    auto per = periodic(rt, 100_ms);
    auto perTask = per.invoking<MockHandler, &MockHandler::invoke>(perHandler);

    loopTasks(power, idleTask, sleepStandbyTask, perTask, basicTask);

    EXPECT_TRUE(power.haveSlept);
    EXPECT_TRUE(idleTask.haveLooped);
    EXPECT_FALSE(perHandler.invoked); // not yet...

    rt.advance(100_ms);
    loopTasks(power, idleTask, sleepStandbyTask, perTask);

    EXPECT_TRUE(perHandler.invoked); // now!
}

}

