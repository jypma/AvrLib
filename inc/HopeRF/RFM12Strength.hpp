#ifndef RFM12_STRENGTH_H
#define RFM12_STRENGTH_H

#include "BitSet.hpp"
#include "HopeRF/RFM12Status.hpp"

namespace HopeRF {

struct drssi_dec_t {
    uint8_t next_if_below_threshold;
    uint8_t next_if_over_threshold;
};

// TODO make this progmen
static constexpr drssi_dec_t drssi_dec_tree[6] = {
        {0, 0},
    /* 1 */ {0 | 0xF0, 2},
    /* 2 */ {1 | 0xF0, 2 | 0xF0},
    /* 3 */ {1, 5},
    /* 4 */ {3 | 0xF0, 4 | 0xF0},
    /* 5 */ {4, 5 | 0xF0}
};

// TODO make this progmem
static constexpr int8_t drssi_strength_table[] = {-106, -100, -94, -88, -82, -76, -70};

/**
 * Calculates the signal strength during reception of an RFM12 packet.
 */
class RFM12Strength {
public:
    volatile uint8_t position = 3;

public:
    /**
     * Applies the given RFM12 status for the next received byte.
     * Returns the value for the next 0x94A0 command to apply.
     */
    uint8_t apply(const BitSet<RFM12Status> status) {
        if (position < 6) { // not yet final value
            if (status[RFM12Status::RSSI_OVER_THRESHOLD])
                position = drssi_dec_tree[position].next_if_over_threshold;
            else
                position = drssi_dec_tree[position].next_if_below_threshold;
        }
        return (position < 6) ? position : 0;
    }

    /**
     * Resets the strength calculator for reception of the next packet.
     * Returns the value for the next 0x94A0 command to apply.
     */
    uint8_t reset() {
        position = 3;
        return position;
    }

    /**
     * Returns the received signal strength in dBm.
     */
    int8_t getStrength() {
        if (position < 6) { // signal strength not (yet) known
            return 0;
        } else {
            return drssi_strength_table[position & 0b111];
        }
    }
};

}

#endif
