/*
 * RFM12_OOK.hpp
 *
 *  Created on: Sep 3, 2015
 *      Author: jan
 */

#ifndef HOPERF_RFM12_OOK_HPP_
#define HOPERF_RFM12_OOK_HPP_

#include "AtomicScope.hpp"

template <typename spi_t,
          typename ss_pin_t>
class RFM12_OOK {
    typedef RFM12_OOK<spi_t, ss_pin_t> This;

    spi_t *spi;
    ss_pin_t *ss_pin;

    void command(uint16_t cmd) {
        ss_pin->setLow();
        spi->send(cmd >> 8);
        spi->send(cmd);
        ss_pin->setHigh();
    }

    void initialize() {
        ss_pin->configureAsOutput();
        ss_pin->setHigh();

        AtomicScope _;
        command(0x8027); // 868 Mhz;disable tx register; disable RX fifo buffer; xtal cap 12pf
        command(0x82c0); // enable receiver ; enable basebandblock        = 430.2400 - 439.7575
        command(0xa684); // 868.3500 MHz exact frequency     a060 - af3f = 860.4800 - 879.5150
        command(0xc691); // datarate 2395 kbps 0xc647 = 4.8kbps
        command(0x9489); // VDI; FAST;200khz;GAIN -6db ;DRSSI 97dbm
        command(0xC220); // datafiltercommand ; ** not documented command **
        command(0xCA00); // FiFo and resetmode command ; FIFO fill disabeld
        command(0xC473); // AFC run only once ; enable AFC ;enable frequency offset register ; +3 -4
        command(0xCC67); // pll settings command
        command(0xB800); // TX register write command NOT USED
        command(0xC800); // low dutycycle disabeld
        command(0xC040); // 1.66MHz,2.2V NOT USED see 82c0
    }

public:
    RFM12_OOK(spi_t &_spi, ss_pin_t &_ss_pin): spi(&_spi), ss_pin(&_ss_pin) {
        initialize();
    }
};

template <typename spi_t,
          typename ss_pin_t>
RFM12_OOK<spi_t, ss_pin_t> rfm12_ook(spi_t &spi, ss_pin_t &ss_pin) {
    return RFM12_OOK<spi_t, ss_pin_t>(spi, ss_pin);
}

#endif /* HOPERF_RFM12_OOK_HPP_ */
