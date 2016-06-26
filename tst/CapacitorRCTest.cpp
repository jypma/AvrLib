/*
 * CapacitorRCTest.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: jan
 */
#include <gtest/gtest.h>
#include "Passive/CapacitorRC.hpp"
#include "Mocks.hpp"
#include "invoke.hpp"

namespace CapacitorRCTest {

using namespace Mocks;
using namespace Passive;

TEST(CapacitorRCTest, should_charge_and_discharge) {
    MockRealTimer rt;
    MockPin pin;
    auto cap = CapacitorRC<10000>(rt, pin);
    cap.measure();
    invoke<MockPin::INT>(cap);
}

}


