#include <gtest/gtest.h>
#include "EEPROMTest.hpp"
#include "Fifo.hpp"

namespace EEPROMTest {

TEST(EEPROMTest, can_read_uint8_t) {
    eeprom_contents[0] = 42;

    Fifo<200> f;
    f.out() << &EEPROM::data;

    EXPECT_EQ(1, f.getSize());
    EXPECT_EQ(42, f.peek());
}

TEST(EEPROMTest, reads_null_terminated_string) {
    eeprom_contents[1] = 42;
    eeprom_contents[2] = 0;

    Fifo<200> f;
    f.out() << &EEPROM::name;

    EXPECT_EQ(1, f.getSize());
    EXPECT_EQ(42, f.peek());
}

TEST(EEPROMTest, doesnt_read_past_end_of_string_when_null_terminator_is_missing) {
    eeprom_contents[1] = 42;
    eeprom_contents[2] = 42;
    eeprom_contents[3] = 42;
    eeprom_contents[4] = 42;
    eeprom_contents[5] = 42;
    eeprom_contents[6] = 42;
    eeprom_contents[7] = 42;
    eeprom_contents[8] = 42;
    eeprom_contents[9] = 42;
    eeprom_contents[10] = 42;

    Fifo<200> f;
    f.out() << &EEPROM::name;

    EXPECT_EQ(10, f.getSize());
}

}
