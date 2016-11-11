#include <gtest/gtest.h>
#include "ROHM/BH1750.hpp"
#include "Mocks.hpp"

namespace BH1750Test {

using namespace ROHM;
using namespace Mocks;

TEST(BH1750Test, can_compile) {
    auto rt = MockRealTimer();
    auto twi = MockTWI();
    auto bh = bh1750(twi, rt);
    bh.measure(BH1750Mode::continuousHighRes);
    bh.getLightLevel();
}

}
