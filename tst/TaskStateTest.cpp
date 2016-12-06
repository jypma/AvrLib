#include <gtest/gtest.h>
#include "Tasks/TaskState.hpp"
#include "Time/RealTimer.hpp"
#include "Mocks.hpp"

namespace TaskStateTest {

using namespace Mocks;
using namespace Time;

TEST(TaskStateTest, should_make_TaskState_of_Ticks_if_deadline_returns_ticks) {
	auto rt = MockRealTimerPrescaled<1>();
	auto d = deadline(rt, 3000_s);
	// let's be sure that the deadline is in ticks, not counts
	Ticks<> check1 = d.timeLeft();
	EXPECT_EQ(93750000, check1.getValue());

	// the returned TaskState should also be in ticks
	auto state = TaskState(d, SleepMode::IDLE);
	Ticks<> check2 = state.timeLeft();
	EXPECT_EQ(93750000, check2.getValue());
}

}
