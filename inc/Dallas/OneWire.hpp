#ifndef DALLAS_ONEWIRE_HPP_
#define DALLAS_ONEWIRE_HPP_

#include <Time/Units.hpp>
#include "Logging.hpp"

namespace Dallas {

struct OneWireAddress {
    uint8_t addr[8];
};

namespace Impl {

using namespace Streams;
using namespace Time;

template<typename pin_t, typename _rt_t>
class OneWire {
    typedef Logging::Log<Loggers::Dallas> log;
    typedef OneWire<pin_t, _rt_t> This;

public:
    typedef _rt_t rt_t;
    rt_t * const rt;

private:
    pin_t * const pin;
    const bool power;

    // TODO refactor out search into a class so we can use these bytes only on the stack while actually searching
    bool LastDeviceFlag = false;
    uint8_t LastDiscrepancy = 0;
    uint8_t LastFamilyDiscrepancy = 0;
    uint8_t ROM_NO[8];

public:
    OneWire(pin_t &p, rt_t &r, bool _power): rt(&r), pin(&p), power(_power) {}

    /**
     * Resets the one-wire devices on the bus, and returns whether at least one device was detected.
     */
    bool reset() {
        uint8_t retries = 125;
        pin->configureAsInputWithPullup();
        // wait until the wire is high... just in case
        do {
            if (--retries == 0) {
                log::debug(F("reset: wire didn't get high"));
                return false;
            }
            delay(2_us);
        } while (pin->isLow());

        {
            AtomicScope _;
            pin->configureAsOutputLow();
        }
        rt->delay(480_us);
        pin->configureAsInputWithPullup();
        delay(35_us);
        delay(35_us);
        bool present = pin->isLow();
        rt->delay(410_us);
        log::debug(F("present: "), dec(uint8_t(present)));
        return present;
    }
#define FALLBACK
    void write_bit(bool b) {
        if (b) {
            {
                AtomicScope _;
                pin->configureAsOutputLow();
                delay(10_us);
                pin->setHigh();
#ifdef FALLBACK
                delay(25_us);
                delay(30_us);
            }
#else
            }
            rt->delay(45_us);
#endif
        } else {
            {
                AtomicScope _;
                pin->configureAsOutputLow();
#ifdef FALLBACK
                delay(32_us);
                delay(33_us);
            }
#else
            }
            rt->delay(55_us);
#endif
            pin->setHigh();
            delay(5_us);
        }
    }

    void write(uint8_t b) {
        uint8_t bitMask;

        for (bitMask = 0x01; bitMask; bitMask <<= 1) {
            write_bit( (bitMask & b) ? 1 : 0);
        }

        if (!power) {
            AtomicScope _;
            pin->configureAsInputWithPullup();
        }
    }

    bool read_bit() {
        bool result;
        {
            AtomicScope _;
            pin->configureAsOutputLow();
            delay(3_us);
            pin->configureAsInputWithPullup();
            delay(10_us);
            result = pin->isHigh();
#ifdef FALLBACK
            delay(23_us);
            delay(30_us);
        }
#else
        }
        rt->delay(43_us);
#endif
        return result;
    }

    uint8_t read() {
        uint8_t r = 0;
        for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
            if (read_bit()) r |= bitMask;
        }
        log::debug(F("read: "), Hexadecimal(r));
        return r;
    }

    void select(const OneWireAddress &addr)
    {
        write(0x55);           // Choose ROM
        for (uint8_t i = 0; i < 8; i++) write(addr.addr[i]);
    }

    /**
     * Searches for a OneWire device connected to the pin, and stores its address in [addr].
     * Repeated calls to search() will return further connected devices, always in the same order.
     *
     * Returns false after the last device has been detected.
     */
    bool search(OneWireAddress &addr) {
        uint8_t id_bit_number;
        uint8_t last_zero, rom_byte_number, search_result;
        uint8_t id_bit, cmp_id_bit;

        uint8_t rom_byte_mask, search_direction;

        // initialize for search
        id_bit_number = 1;
        last_zero = 0;
        rom_byte_number = 0;
        rom_byte_mask = 1;
        search_result = 0;

        // if the last call was not the last one
        if (!LastDeviceFlag)
        {
           // 1-Wire reset
           if (!reset())
           {
              // reset the search
              LastDiscrepancy = 0;
              LastDeviceFlag = false;
              LastFamilyDiscrepancy = 0;
              return false;
           }

           // issue the search command
           //if (search_mode == true) {
             write(0xF0);   // NORMAL SEARCH
           //} else {
           //  write(0xEC);   // CONDITIONAL SEARCH
           //}

           // loop to do the search
           do
           {
              // read a bit and its complement
              id_bit = read_bit();
              cmp_id_bit = read_bit();

              // check for no devices on 1-wire
              if ((id_bit == 1) && (cmp_id_bit == 1))
                 break;
              else
              {
                 // all devices coupled have 0 or 1
                 if (id_bit != cmp_id_bit)
                    search_direction = id_bit;  // bit write value for search
                 else
                 {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                       search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                       // if equal to last pick 1, if not then pick 0
                       search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0)
                    {
                       last_zero = id_bit_number;

                       // check for Last discrepancy in family
                       if (last_zero < 9)
                          LastFamilyDiscrepancy = last_zero;
                    }
                 }

                 // set or clear the bit in the ROM byte rom_byte_number
                 // with mask rom_byte_mask
                 if (search_direction == 1)
                   ROM_NO[rom_byte_number] |= rom_byte_mask;
                 else
                   ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                 // serial number search direction write bit
                 write_bit(search_direction);

                 // increment the byte counter id_bit_number
                 // and shift the mask rom_byte_mask
                 id_bit_number++;
                 rom_byte_mask <<= 1;

                 // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                 if (rom_byte_mask == 0)
                 {
                     rom_byte_number++;
                     rom_byte_mask = 1;
                 }
              }
           }
           while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

           // if the search was successful then
           if (!(id_bit_number < 65))
           {
              // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
              LastDiscrepancy = last_zero;

              // check for last device
              if (LastDiscrepancy == 0)
                 LastDeviceFlag = true;

              search_result = true;
           }
        }

        // if no device found then reset counters so next 'search' will be like a first
        if (!search_result || !ROM_NO[0])
        {
           log::debug(F("search: no device found "));
           LastDiscrepancy = 0;
           LastDeviceFlag = false;
           LastFamilyDiscrepancy = 0;
           search_result = false;
        } else {
           for (int i = 0; i < 8; i++) addr.addr[i] = ROM_NO[i];
        }
        return search_result;
    }
};

} // namespace Impl

template<typename pin_t, typename rt_t>
Impl::OneWire<pin_t, rt_t> OneWireParasitePower(pin_t &p, rt_t &rt) {
    return { p, rt, true };
}

template<typename pin_t, typename rt_t>
Impl::OneWire<pin_t, rt_t> OneWireUnpowered(pin_t &p, rt_t &rt) {
    return { p, rt, false };
}

} // namespace Dallas

#endif /* DALLAS_ONEWIRE_HPP_ */
