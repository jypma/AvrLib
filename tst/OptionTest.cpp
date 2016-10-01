#include "Option.hpp"
#include <gtest/gtest.h>

TEST(OptionTest, can_map) {
	Option<int> o;

	o = 42;
	auto o2 = o.map([] (auto i) { return i == 0; });

	EXPECT_TRUE(o2.isDefined());
	EXPECT_FALSE(o2.isEmpty());
	EXPECT_EQ(false, o2.get());

	Option<int> n;
	EXPECT_EQ(42, n.getOrElse(42));
	EXPECT_EQ(42, o.getOrElse(0));
}
