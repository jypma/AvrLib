#pragma once

/*
Arduino:
TWBR: 72
TWCR: 69
TWSR: 248
TWDR: 255
TWAR: 254
TWAMR: 0

AvrLin (before init TSL)
Fri 14 Dec 2018 08:33:20 PM CET:  M: RoomSensor
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWBR: 72
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWCR: 69
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWSR: 248
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWDR: 255
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWAR: 254
Fri 14 Dec 2018 08:33:20 PM CET:  M: TWAMR: 0
jan@jinkpad:~/Documents/workspace-arduino/AvrLib

AvrLib (after init TSL)
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWBR: 72
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWCR: 69
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWSR: 248
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWDR: 3
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWAR: 254
Thu 13 Dec 2018 09:27:05 PM CET:  M: TWAMR: 0


*/
#include <stdint.h>
#include "Enum.hpp"
#include "Option.hpp"
#include "Time/RealTimer.hpp"
#include "Tasks/TaskState.hpp"
#include "Logging.hpp"

#define auto_field(name, expr) decltype(expr) name = expr

namespace Ambient {

namespace TSL2561_AddressPin {

struct Low {
  constexpr static uint8_t address = 0x29;
};

struct Floating {
  constexpr static uint8_t address = 0x39;
};

struct High {
  constexpr static uint8_t address = 0x49;
};

}

namespace TSL2561_Package {

uint32_t getRatio(uint32_t channel0, uint32_t channel1) {
  constexpr static uint8_t TSL2561_LUX_RATIOSCALE = 9;
  /* Find the ratio of the channel values (Channel1/Channel0) */
  uint32_t ratio1 = 0;
  if (channel0 != 0) {
    ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
  }

  /* round the ratio value */
  return (ratio1 + 1) >> 1;
}

struct ChipScale {
  static constexpr uint16_t TSL2561_LUX_K1C = 0x0043;  ///< 0.130 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B1C = 0x0204;  ///< 0.0315 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M1C = 0x01ad;  ///< 0.0262 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K2C = 0x0085;  ///< 0.260 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B2C = 0x0228;  ///< 0.0337 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M2C = 0x02c1;  ///< 0.0430 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K3C = 0x00c8;  ///< 0.390 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B3C = 0x0253;  ///< 0.0363 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M3C = 0x0363;  ///< 0.0529 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K4C = 0x010a;  ///< 0.520 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B4C = 0x0282;  ///< 0.0392 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M4C = 0x03df;  ///< 0.0605 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K5C = 0x014d;  ///< 0.65 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B5C = 0x0177;  ///< 0.0229 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M5C = 0x01dd;  ///< 0.0291 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K6C = 0x019a;  ///< 0.80 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B6C = 0x0101;  ///< 0.0157 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M6C = 0x0127;  ///< 0.0180 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K7C = 0x029a;  ///< 1.3 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B7C = 0x0037;  ///< 0.00338 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M7C = 0x002b;  ///< 0.00260 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K8C = 0x029a;  ///< 1.3 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B8C = 0x0000;  ///< 0.000 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M8C = 0x0000;  ///< 0.000 * 2^LUX_SCALE

  static uint32_t scale(uint32_t channel0, uint32_t channel1) {
    uint16_t b,m;
    uint32_t ratio = getRatio(channel0, channel1);
    if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1C))
      {b=TSL2561_LUX_B1C; m=TSL2561_LUX_M1C;}
    else if (ratio <= TSL2561_LUX_K2C)
      {b=TSL2561_LUX_B2C; m=TSL2561_LUX_M2C;}
    else if (ratio <= TSL2561_LUX_K3C)
      {b=TSL2561_LUX_B3C; m=TSL2561_LUX_M3C;}
    else if (ratio <= TSL2561_LUX_K4C)
      {b=TSL2561_LUX_B4C; m=TSL2561_LUX_M4C;}
    else if (ratio <= TSL2561_LUX_K5C)
      {b=TSL2561_LUX_B5C; m=TSL2561_LUX_M5C;}
    else if (ratio <= TSL2561_LUX_K6C)
      {b=TSL2561_LUX_B6C; m=TSL2561_LUX_M6C;}
    else if (ratio <= TSL2561_LUX_K7C)
      {b=TSL2561_LUX_B7C; m=TSL2561_LUX_M7C;}
    else
      {b=TSL2561_LUX_B8C; m=TSL2561_LUX_M8C;}
    return ((channel0 * b) - (channel1 * m));
  }
};

struct Other {
  static constexpr uint16_t TSL2561_LUX_K1T = 0x0040;  ///< 0.125 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B1T = 0x01f2;  ///< 0.0304 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M1T = 0x01be;  ///< 0.0272 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K2T = 0x0080;  ///< 0.250 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B2T = 0x0214;  ///< 0.0325 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M2T = 0x02d1;  ///< 0.0440 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K3T = 0x00c0;  ///< 0.375 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B3T = 0x023f;  ///< 0.0351 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M3T = 0x037b;  ///< 0.0544 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K4T = 0x0100;  ///< 0.50 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B4T = 0x0270;  ///< 0.0381 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M4T = 0x03fe;  ///< 0.0624 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K5T = 0x0138;  ///< 0.61 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B5T = 0x016f;  ///< 0.0224 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M5T = 0x01fc;  ///< 0.0310 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K6T = 0x019a;  ///< 0.80 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B6T = 0x00d2;  ///< 0.0128 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M6T = 0x00fb;  ///< 0.0153 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K7T = 0x029a;  ///< 1.3 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B7T = 0x0018;  ///< 0.00146 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M7T = 0x0012;  ///< 0.00112 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_K8T = 0x029a;  ///< 1.3 * 2^RATIO_SCALE
  static constexpr uint16_t TSL2561_LUX_B8T = 0x0000;  ///< 0.000 * 2^LUX_SCALE
  static constexpr uint16_t TSL2561_LUX_M8T = 0x0000;  ///< 0.000 * 2^LUX_SCALE

  static uint32_t scale(uint32_t channel0, uint32_t channel1) {
    uint16_t b,m;
    uint32_t ratio = getRatio(channel0, channel1);
    if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
      {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
    else if (ratio <= TSL2561_LUX_K2T)
      {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
    else if (ratio <= TSL2561_LUX_K3T)
      {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
    else if (ratio <= TSL2561_LUX_K4T)
      {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
    else if (ratio <= TSL2561_LUX_K5T)
      {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
    else if (ratio <= TSL2561_LUX_K6T)
      {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
    else if (ratio <= TSL2561_LUX_K7T)
      {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
    else if (ratio > TSL2561_LUX_K8T)
      {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}
    return ((channel0 * b) - (channel1 * m));
  }
};

}

enum class TSL2561_Time { _13ms, _101ms, _402ms };

using namespace Streams;

template <typename twi_t, typename rt_t, typename addr_pin = TSL2561_AddressPin::Floating, typename package = TSL2561_Package::Other>
class TSL2561 {
  typedef Logging::Log<Loggers::Ambient> log;
  constexpr static uint8_t address = addr_pin::address;

  constexpr static uint8_t TSL2561_REGISTER_TIMING = 0x01;
  constexpr static uint8_t TSL2561_REGISTER_ID = 0x0A;
  constexpr static uint8_t TSL2561_COMMAND_BIT = 0x80;
  constexpr static uint8_t TSL2561_REGISTER_CHAN0_LOW = 0x0C;
  constexpr static uint8_t TSL2561_REGISTER_CHAN1_LOW = 0x0E;
  constexpr static uint8_t TSL2561_WORD_BIT = 0x20;
  constexpr static uint8_t TSL2561_REGISTER_CONTROL = 0x00;
  constexpr static uint8_t TSL2561_CONTROL_POWERON = 0x03;
  constexpr static uint8_t TSL2561_CONTROL_POWEROFF = 0x00;

  constexpr static uint8_t TSL2561_LUX_CHSCALE = 10;
  constexpr static uint8_t TSL2561_LUX_LUXSCALE = 14;

  twi_t *const twi;
  rt_t *const rt;
  auto_field(measurementComplete, deadline(*rt));

  bool initialized = false;
  bool gain = false;
  bool autoGain = true;
  TSL2561_Time integrationTime = TSL2561_Time::_101ms;

  void initialize() {
    twi->write(address, TSL2561_REGISTER_ID);
    uint8_t value = 0;
    if (!twi->read(address, &value)) {
      log::debug(F("!i"));
    } else {
      log::debug(F("i: "), dec(value));
    }
    initialized = (value & 0x0A) != 0;
  }

  void write8(uint8_t reg, uint8_t value) {
    if (!twi->write(address, reg, value)) {
      log::debug(F("!w"));
    }
    twi->flush();
  }

  Option<uint16_t> read16(uint8_t reg) {
    log::debug(F("start read"));
    log::flush();

    if (!twi->write(address, reg)) {
      log::debug(F("error write read16"));
      return none();
    }
    uint8_t hi = 1, lo = 1;

    if (!twi->read(address, &hi, &lo)) {
      log::debug(F("error read16"));
      return none();
    } else {
      log::debug('<', dec(hi), ',', dec(lo));
    }

    return hi << 8 | lo;
  }

  uint16_t getClipThreshold() {
    switch(integrationTime) {
      case TSL2561_Time::_13ms: return 4900;
      case TSL2561_Time::_101ms: return 37000;
      default: return 65000;
    }
  }

  uint32_t getScale() {
    switch(integrationTime) {
      case TSL2561_Time::_13ms:  return 0x7517;   ///< 322/11 * 2^TSL2561_LUX_CHSCALE
      case TSL2561_Time::_101ms: return 0x0FE7;   ///< 322/81 * 2^TSL2561_LUX_CHSCALE
      default: return 1 << TSL2561_LUX_CHSCALE;
    }
  }

  uint32_t calculateLux(uint16_t broadband, uint16_t ir) {
    uint16_t clipThreshold = getClipThreshold();
    if ((broadband > clipThreshold) || (ir > clipThreshold)) {
      return 65536;
    }

    uint32_t chScale = getScale();
    if (gain) {
      chScale <<= 4;
    }

    uint32_t channel0 = (broadband * chScale) >> TSL2561_LUX_CHSCALE;
    uint32_t channel1 = (ir * chScale) >> TSL2561_LUX_CHSCALE;

    uint32_t temp = package::scale(channel0, channel1);
    /* Do not allow negative lux value */
    if (temp < 0) temp = 0;

    /* Round lsb (2^(LUX_SCALE-1)) */
    temp += (1 << (TSL2561_LUX_LUXSCALE-1));

    /* Strip off fractional portion */
    return temp >> TSL2561_LUX_LUXSCALE;
  }

public:
  void disable() {
    write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWEROFF);
  }

  void enable() {
    write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
  }

  void applyTiming() {
    if (!initialized) {
      initialize(); 
    }
    uint8_t _integration = static_cast<uint8_t> (integrationTime);
    uint8_t _gain = (gain) ? 0x10 : 0;
    enable();
    write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, _integration | _gain);
    disable();
  }

public:
  /** Automatically adjusts gain on/off after each measurement, depending on how much light was measured */
  void setAutoGain(bool a) {
    autoGain = a;
  }

  void setGain(bool g) {
    gain = g;
    applyTiming();
  }

  void setIntegrationTime(TSL2561_Time t) {
    integrationTime = t;
    applyTiming();
  }

  TSL2561(twi_t &_twi, rt_t &_rt): twi(&_twi), rt(&_rt) {
    initialize();
    log::debug(F("Init: "), '0' + initialized);
    if (initialized) {
      applyTiming();
    }
  }

  void measure() {
    if (!initialized) {
      log::debug(F("m:i"));
      initialize();
    }
    log::debug(F("m:e"));
    enable();
    switch(integrationTime) {
      case TSL2561_Time::_13ms:  measurementComplete.schedule(14_ms); break;
      case TSL2561_Time::_101ms: measurementComplete.schedule(102_ms); break;
      case TSL2561_Time::_402ms: measurementComplete.schedule(403_ms); break;
    }
  }

  bool isMeasuring() {
    if (measurementComplete.isNow()) {
      return false;
    }
    return measurementComplete.isScheduled();
  }

  bool isIdle() {
    return !isMeasuring();
  }

  auto timeLeft() const {
    return measurementComplete.timeLeft();
  }

  auto getTaskState() const {
    return TaskState(measurementComplete.timeLeftIfScheduled(), SleepMode::POWER_DOWN);
  }

  Option<uint32_t> getLightLevel() {
    if (isMeasuring() || !initialized) {
      return none();
    }

    const auto broadband = read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);
    const auto ir = read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);

    disable();

    if (broadband.isEmpty() || ir.isEmpty()) {
      return none();
    }

    const auto result = calculateLux(broadband.get(), ir.get());

    if (autoGain) {
      uint16_t lo, hi;
      switch(integrationTime) {
        case TSL2561_Time::_13ms:
          hi = 4850;
          lo = 100;
          break;
        case TSL2561_Time::_101ms:
          hi = 36000;
          lo = 200;
          break;
        case TSL2561_Time::_402ms:
          hi = 63000;
          lo = 500;
          break;
      }
      if (broadband.get() < lo && !gain) {
        setGain(true);
      } else if (broadband.get() > hi && gain) {
        setGain(false);
      }
    }

    return result;
  }
};

template <typename twi_t, typename rt_t, typename addr_pin = TSL2561_AddressPin::Floating>
TSL2561<twi_t,rt_t,addr_pin> tsl2561(twi_t &twi, rt_t &rt) {
    return { twi, rt };
}

}
