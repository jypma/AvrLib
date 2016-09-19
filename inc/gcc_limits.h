#ifndef __AVR__
#include <limits>
#else

#ifndef GCC_LIMITS_H
#define GCC_LIMITS_H

#include <stdint.h>

namespace std {

  template<typename _Tp>
    struct numeric_limits
    {

    };

  template<>
  struct numeric_limits<uint8_t>
  {
     static constexpr uint8_t max() { return 255; }
     static constexpr uint8_t min() { return 0; }
  };

  template<>
  struct numeric_limits<uint16_t>
  {
      static constexpr uint16_t max() { return 65535ll; }
      static constexpr uint16_t min() { return 0; }
  };

  template<>
  struct numeric_limits<uint32_t>
  {
      static constexpr uint32_t max() { return 4294967295ll; }
      static constexpr uint32_t min() { return 0; }
  };

  template<>
  struct numeric_limits<uint64_t>
  {
      static constexpr uint64_t max() { return 18446744073709551615UL; }
      static constexpr uint64_t min() { return 0; }
};

  template<>
  struct numeric_limits<int8_t>
  {
     static constexpr int8_t max() { return 127; }
     static constexpr int8_t min() { return -128; }
  };

  template<>
  struct numeric_limits<int16_t>
  {
      static constexpr int16_t max() { return 32767ll; }
      static constexpr int16_t min() { return -32768ll; }
  };

  template<>
  struct numeric_limits<int32_t>
  {
      static constexpr int32_t max() { return 2147483647ll; }
      static constexpr int32_t min() { return -2147483648ll; }
  };

  template<>
  struct numeric_limits<int64_t>
  {
      static constexpr int64_t max() { return 9223372036854775807ll; }
      static constexpr int64_t min() { return -9223372036854775807ll; }
  };

}

#endif

#endif
