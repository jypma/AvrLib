#ifndef __AVR__
#include <limits>
#else

#ifndef GCC_LIMITS_H

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
      static constexpr uint16_t max() { return 65535; }
  };

}

#endif

#endif
