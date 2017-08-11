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

TEST(OptionTest, can_use_Option_in_range_for_loop) {
    Option<int> o = 42;

    bool invoked = false;
    for (int i: o) {
        invoked = true;
        EXPECT_EQ(42, i);
    }
    EXPECT_TRUE(invoked);
}

TEST(OptionTest, can_use_None_in_range_for_loop) {
    Option<int> o = none();

    bool invoked = false;
    for (int i: o) {
        invoked = true;
    }
    EXPECT_FALSE(invoked);
}
