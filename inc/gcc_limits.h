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
  };

  template<>
  struct numeric_limits<uint16_t>
  {
      static constexpr uint16_t max() { return 65535ll; }
  };

  template<>
  struct numeric_limits<uint32_t>
  {
      static constexpr uint32_t max() { return 4294967295ll; }
  };

  template<>
  struct numeric_limits<uint64_t>
  {
      static constexpr uint64_t max() { return 9223372036854775807ll; }
  };

}

#endif

#endif
