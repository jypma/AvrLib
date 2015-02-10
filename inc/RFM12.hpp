/*
 * RFM12.hpp
 *
 *  Created on: Jan 17, 2015
 *      Author: jan
 */

#ifndef RFM12_HPP_
#define RFM12_HPP_

#include "Pin.hpp"
#include "SPI.hpp"
#include "Interrupt.hpp"
#include "BitSet.hpp"
#include "Fifo.hpp"
#include "CRC.hpp"
#include "ChunkedFifo.hpp"
#include "RFM12TxFifo.hpp"
#include "RFM12RxFifo.hpp"

enum class RFM12Band: uint8_t {
    _433MHz = 1, _868Mhz = 2, _915MHz = 3
};

struct drssi_dec_t {
    uint8_t up;
    uint8_t down;
    uint8_t threshold;
};

static constexpr drssi_dec_t drssi_dec_tree[6] = {
    /* up down thres*/
    /* 0 */ { 0b1001, 0b1000, 0b000 }, /* B1xxx show final values, B0xxx are intermediate */
    /* 1 */ { 0b0010, 0b0000, 0b001 }, /* values where next threshold has to be set. */
    /* 2 */ { 0b1011, 0b1010, 0b010 }, /* Traversing of this three is in rf_12interrupt() */
    /* 3 */ { 0b0101, 0b0001, 0b011 }, // <- start value
    /* 4 */ { 0b1101, 0b1100, 0b100 },
    /* 5 */ { 0b1110, 0b0100, 0b101 }
};

template <const SPIMaster &spi, ChunkedFifo &_txFifo, ChunkedFifo &_rxFifo, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin>
class RFM12 {
    static void command(uint16_t cmd) {
        ss_pin.setLow();
        spi.send(cmd >> 8);
        spi.send(cmd);
        ss_pin.setHigh();
    }

    static void commandIdle() {
        command(0x820D);  // RF_IDLE_MODE
    }

    enum class State {
       TXCRC1, TXCRC2, TXTAIL, TXDONE, TXIDLE,
       TXRECV,
       TXPRE1, TXPRE2, TXPRE3, TXSYN1, TXSYN2,
    };

    enum class Status: uint16_t {
        // Note that the bit values are in reverse w.r.t. the datasheet, since we
        // read the status word with RGIT/FFIT as MSB.
        READY_FOR_NEXT_BYTE = uint16_t(1) << 15,  // RGIT (when sending) or FFIT (when receiving)
        POWER_ON_RESET      = 1 << 14,  // POR
        UNDERRUN_OVERFLOW   = 1 << 13,  // RGUR (when sending) or FFOv (when receiving)
        RSSI_OVER_THRESHOLD = 1 << 8    // ATG (when sending) or RSSI (when receiving)
    };

    class DRSSI {
        volatile uint8_t position = 3;

    public:
        void apply(BitSet<Status> status) {
            if (position < 6) { // not yet final value
                if (status[Status::RSSI_OVER_THRESHOLD])
                    position = drssi_dec_tree[position].up;
                else
                    position = drssi_dec_tree[position].down;
                if (position < 6) { // not yet final destination
                    RFM12::command(0x94A0 | drssi_dec_tree[position].threshold);
                }
            }
        }

        void reset() {
            position = 3;
            RFM12::command(0x94A0 | drssi_dec_tree[position].threshold);
        }
    };

public:
    volatile State state = State::TXIDLE;
    DRSSI drssi;
    RFM12TxFifo txFifo; // maybe just expose the Reader and Writer interfaces?
    RFM12RxFifo rxFifo; // maybe just expose the Reader and Writer interfaces?
    CRC16 crc;

    BitSet<Status> getStatus(uint8_t &in) {
        ss_pin.setLow();
        // status bits are transmitted by RFM12 when sending 0x0000.
        uint16_t statusBits = spi.transceive(0x00) << 8;
        statusBits |= spi.transceive(0x00);
        auto res = BitSet<Status>(statusBits);

        if (res[Status::READY_FOR_NEXT_BYTE] && state == State::TXRECV) {
            // RFM12's FIFO has a byte for us
            // slow down to under 2.5 MHz
            spi.setClockPrescaler(SPI2MHz);
            in = spi.transceive(0x00);
            spi.setClockPrescaler(SPI8MHz);
        }

        ss_pin.setHigh();
        return res;
    }
    volatile uint8_t ints = 0;
    volatile uint8_t lastLen = 0;
    volatile uint8_t recvCount = 0;
    volatile uint8_t underruns = 0;

    void interrupt() {
        ints++;
        uint8_t in = 0;
        auto status = getStatus(in);
        if (status[Status::READY_FOR_NEXT_BYTE]) {
            if (state == State::TXRECV) {
                //TODO consider moving the spi read block in here, to not have the double if.
                recvCount++;

                drssi.apply(status);

                if (rxFifo.isWriting()) {
                    rxFifo.write(in);
                } else {
                    lastLen = in;
                    rxFifo.writeStart(in);
                }

                if (!rxFifo.isWriting()) {
                    // fifo expects no further bytes -> either we just received the last byte, or length was 0
                    commandIdle();

                    // TODO decide to go to sending mode here instead, if txfifo has stuff in it.
                    listen();
                }
            } else {
                // we are sending
                if (txFifo.isReading()) {
                    uint8_t b;
                    txFifo.read(b);
                    command(0xB800 | b); // RF_TXREG_WRITE
                } else {
                    txFifo.readEnd();
                    commandIdle();
                    command(0xB8AA);     // RF_TXREG_WRITE 0xAA (postfix)
                }
            }
        }

        // power-on reset
        if (status[Status::POWER_ON_RESET]) {
            // TODO make this actually work...
        }

        // fifo overflow or buffer underrun - abort reception/sending
        if (status[Status::UNDERRUN_OVERFLOW]) {
            underruns++;
            rxFifo.writeAbort();
            txFifo.readAbort();
            RFM12::commandIdle();
            state = State::TXIDLE;
        }
    }

    static void onInterrupt(volatile void *rfm) {
        ((RFM12*)rfm)->interrupt();
    }

public:
    RFM12(RFM12Band band): txFifo(&_txFifo), rxFifo(&_rxFifo, false) {
        enable(band); // make this optional by template maybe?
    }

    void enable(RFM12Band band) {
        cli();
        ss_pin.configureAsOutput();
        ss_pin.setHigh();
        int_pin.configureAsInputWithPullup();
        int_pin.interruptOnExternal().attach(&RFM12::onInterrupt, this);
        int_pin.interruptOnExternalLow();


        command(0x0000); // initial SPI transfer added to avoid power-up problem
        command(0x8205); // RF_SLEEP_MODE: DC (disable clk pin), enable lbd

        // wait until RFM12B is out of power-up reset, this takes several *seconds*
        command(0xB800); // RF_TXREG_WRITE in case we're still in OOK mode
        while (int_pin.isLow()) {
            command(0x0000);
        }

        command(0x80C7 | (static_cast<uint8_t>(band) << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
        command(0xA640); // 868MHz
        command(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
        command(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
        command(0xC2AC); // AL,!ml,DIG,DQD4
        command(0xCA8B); // FIFO8,1-SYNC,!ff,DR
        command(0xCE2D); // SYNC=2D；
        command(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
        command(0x9850); // !mp,90kHz,MAX OUT
        command(0xCC77); // OB1，OB0, LPX,！ddy，DDIT，BW0
        command(0xE000); // NOT USE
        command(0xC800); // NOT USE
        command(0xC049); // 1.66MHz,3.1V
        state = State::TXIDLE;

        sei();
    }

    /**
     * Aborts any transmission (or receive) in progress, and starts listening for packets.
     */
    void listen() { // rf12_recvStart
        AtomicScope _;

        crc.reset();
        drssi.reset();
        state = State::TXRECV;
        command(0x82DD); // RF_RECEIVER_ON
    }

    void onTxFifoWriteComplete() {
        state = State::TXPRE1;
        txFifo.readStart();
        command(0x823D); // RF_XMITTER_ON
    }

    inline Reader in() {
        return rxFifo.in();
    }

    inline bool hasContent() const {
        return rxFifo.hasContent();
    }

    class OOK {
    public:
        OOK() {
            int_pin.interruptOnExternalOff();
        }
        ~OOK() {
            int_pin.interruptOnExternalLow();
        }
        void on() {
            RFM12::command(0x823D); // RF_XMITTER_ON
        }
        void off() {
            RFM12::commandIdle();
        }
    };

    OOK ook() {
        return OOK();
    }


};



#endif /* RFM12_HPP_ */
