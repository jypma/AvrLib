#include <gtest/gtest.h>
#include "HAL/Bit.hpp"
#include "HAL/Register8.hpp"

namespace BitTest {

using namespace HAL;

using Reg = Register8<42,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr Reg::Bit0 Bit0 = {};
constexpr Reg::Bit1 Bit1 = {};
constexpr Reg::Bit2 Bit2 = {};
constexpr Reg::Bit3 Bit3 = {};
constexpr Reg::Bit4 Bit4 = {};
constexpr Reg::Bit5 Bit5 = {};
constexpr Reg::Bit6 Bit6 = {};
constexpr Reg::Bit7 Bit7 = {};

TEST(BitTest, apply_should_set_several_bits) {
  Reg reg = {};
  reg.set(0xF0);

  reg.apply(Bit0 | Bit1);

  EXPECT_EQ(0xF3, reg.get());
}

TEST(BitTest, apply_should_clear_several_bits) {
  Reg reg = {};
  reg.set(0xF3);

  reg.apply(~Bit0 | ~Bit1);

  EXPECT_EQ(0xF0, reg.get());
}

TEST(BitTest, apply_should_set_one_bit_and_clear_another) {
  Reg reg = {};
  reg.set(0xF1);

  reg.apply(~Bit0 | Bit1);

  EXPECT_EQ(0xF2, reg.get());
}

TEST(BitTest, operator_can_clear_single_bit) {
  Reg reg = {};
  reg.set(0xF1);

  reg &= ~Bit0;

  EXPECT_EQ(0xF0, reg.get());
}

TEST(BitTest, operator_can_clear_multiple_bits) {
  Reg reg = {};
  reg.set(0xF3);

  reg &= ~Bit0 & ~Bit1;

  EXPECT_EQ(0xF0, reg.get());

  reg.set(0xF3);

  reg &= ~(Bit0 | Bit1);

  EXPECT_EQ(0xF0, reg.get());
}

TEST(BitTest, operator_can_set_single_bit) {
  Reg reg = {};
  reg.set(0xF0);

  reg |= Bit0;

  EXPECT_EQ(0xF1, reg.get());
}

TEST(BitTest, operator_can_set_multiple_bits) {
  Reg reg = {};
  reg.set(0xF0);

  reg |= Bit0 | Bit1;

  EXPECT_EQ(0xF3, reg.get());
}

TEST(BitTest, can_select_several_bits_into_an_int) {
  Reg reg = {};
  reg.set(0xF3);

  uint8_t i = reg.get() & (Bit0 | Bit1);

  EXPECT_EQ(3, i);
}

}
