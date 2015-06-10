#ifndef RFM12STATUS_HPP_
#define RFM12STATUS_HPP_

namespace HopeRF {

enum class RFM12Status: uint16_t {
    // Note that the bit values are in reverse w.r.t. the datasheet, since we
    // read the status word with RGIT/FFIT as MSB.
    READY_FOR_NEXT_BYTE = uint16_t(1) << 15,  // RGIT (when sending) or FFIT (when receiving)
    POWER_ON_RESET      = uint16_t(1) << 14,  // POR
    UNDERRUN_OVERFLOW   = uint16_t(1) << 13,  // RGUR (when sending) or FFOv (when receiving)
    RSSI_OVER_THRESHOLD = uint16_t(1) << 8    // ATG (when sending) or RSSI (when receiving)
};

}

#endif /* RFM12STATUS_HPP_ */
