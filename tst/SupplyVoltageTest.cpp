#include <gtest/gtest.h>
#include "EEPROMTest.hpp"
#include "Passive/SupplyVoltage.hpp"

namespace SupplyVoltageTest {

using namespace Passive;

struct MockADC {
    uint16_t value = 0;

    void setReference(ADReference ref) {}
    template<typename pin_t> void start() {}
    uint16_t awaitValue() { return value; }
};

struct MockPin {

};

TEST(SupplyVoltageTest, boundary_voltages_are_calculated_correctly) {
    MockADC adc;
    MockPin pin;
    constexpr uint16_t R1 = 10000;
    constexpr uint16_t R2 = 680;
    auto voltage = SupplyVoltage<R1, R2, &EEPROM::bandgapVoltage>(adc, pin);
    eeprom_set(&EEPROM::bandgapVoltage, 1100);

    adc.value = 0;
    EXPECT_EQ(0, voltage.get());
    adc.value = 1023;
    EXPECT_EQ(17245, voltage.get());
}

}
