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

template <const SPIMaster &spi, ChunkedFifo &_txFifo, ChunkedFifo &_rxFifo, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin>
class RFM12 {
    static void command(uint16_t cmd) {
        ss_pin.setLow();
        spi.send(cmd >> 8);
        spi.send(cmd);
        ss_pin.setHigh();
    }

    static const Writer::VTable writerVTable;

public:
    enum class Mode { IDLE, LISTENING, RECEIVING, SENDING };

private:
    enum class Status: uint16_t {
        // Note that the bit values are in reverse w.r.t. the datasheet, since we
        // read the status word with RGIT/FFIT as MSB.
        READY_FOR_NEXT_BYTE = uint16_t(1) << 15,  // RGIT (when sending) or FFIT (when receiving)
        POWER_ON_RESET      = uint16_t(1) << 14,  // POR
        UNDERRUN_OVERFLOW   = uint16_t(1) << 13,  // RGUR (when sending) or FFOv (when receiving)
        RSSI_OVER_THRESHOLD = uint16_t(1) << 8    // ATG (when sending) or RSSI (when receiving)
    };

    class DRSSI {
    public:
        volatile uint8_t position = 3;

    public:
        void apply(const BitSet<Status> status) {
            if (position < 6) { // not yet final value
                if (status[Status::RSSI_OVER_THRESHOLD])
                    position = drssi_dec_tree[position].next_if_over_threshold;
                else
                    position = drssi_dec_tree[position].next_if_below_threshold;
                if (position < 6) { // not yet final value
                    RFM12::command(0x94A0 | position);
                }
            }
        }

        void reset() {
            position = 3;
            RFM12::command(0x94A0 | position);
        }

        int8_t getStrength() {
            if (position < 6) { // signal strength not (yet) known
                return 0;
            } else {
                return drssi_strength_table[position & 0b111];
            }
        }
    };

public:
    volatile Mode mode = Mode::IDLE;
    DRSSI drssi;
    RFM12TxFifo txFifo;
    RFM12RxFifo rxFifo;

    BitSet<Status> getStatus(uint8_t &in) {
        ss_pin.setLow();
        // status bits are transmitted by RFM12 when sending 0x0000.
        uint16_t statusBits = spi.transceive(0x00) << 8;
        statusBits |= spi.transceive(0x00);
        auto res = BitSet<Status>(statusBits);

        if (res[Status::READY_FOR_NEXT_BYTE] && (mode == Mode::RECEIVING || mode == Mode::LISTENING)) {
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
    volatile int8_t lastStrength = 0;
    volatile uint8_t recvCount = 0;
    volatile uint8_t underruns = 0;
    volatile uint8_t idx = 0;

    void commandIdle() {
        command(0x820D);  // RF_IDLE_MODE
        mode = Mode::IDLE;
    }

    void sendOrListen() { // rf12_recvStart
        AtomicScope _;

        if (mode == Mode::IDLE) {
            drssi.reset();
            rxFifo.writeAbort();
            if (txFifo.hasContent()) {
                mode = Mode::SENDING;
                txFifo.readStart();
                command(0x823D); // RF_XMITTER_ON
            } else {
                mode = Mode::LISTENING;
                command(0x82DD); // RF_RECEIVER_ON
            }
        }
    }

    void interrupt() {
        ints++;
        uint8_t in = 0;
        auto status = getStatus(in);
        if (status[Status::READY_FOR_NEXT_BYTE]) {
            if (mode == Mode::SENDING) {
                // we are sending
                if (txFifo.isReading()) {
                    uint8_t b;
                    txFifo.read(b);
                    command(0xB800 | b); // RF_TXREG_WRITE
                } else {
                    txFifo.readEnd();
                    commandIdle();
                    command(0xB8AA);     // RF_TXREG_WRITE 0xAA (postfix)
                    sendOrListen();
                }
            } else {
                //TODO consider moving the spi read block in here, to not have the double if.
                recvCount++;

                drssi.apply(status);
                lastStrength = drssi.getStrength();

                //if (status[Status::RSSI_OVER_THRESHOLD]) {
                //    lastStrength++;
               // }

                if (rxFifo.isWriting()) {
                    rxFifo.write(in);
                    idx++;
                    if (idx >= 5) idx = 5;
                    RFM12::command(0x94A0 | idx);
                } else {
                    lastLen = in;
                    idx = 0;
                    rxFifo.writeStart(in);
                    mode = Mode::RECEIVING;
                    RFM12::command(0x94A0 | idx);
                }

                if (!rxFifo.isWriting()) {
                    // fifo expects no further bytes -> either we just received the last byte, or length was 0
                    commandIdle();
                    sendOrListen();
                }
            }
        }

        // power-on reset
        if (status[Status::POWER_ON_RESET]) {
            commandIdle();
            sendOrListen();
        }

        // fifo overflow or buffer underrun - abort reception/sending
        if (status[Status::UNDERRUN_OVERFLOW]) {
            underruns++;
            rxFifo.writeAbort();
            txFifo.readAbort();
            commandIdle();
            sendOrListen();
        }
    }

    static void onInterrupt(volatile void *rfm) {
        ((RFM12*)rfm)->interrupt();
    }

    static void writeStart(void *ctx) {
        ((RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>*)(ctx))->txFifo.writeStart();
    }
    static void writeEnd(void *ctx) {
        ((RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>*)(ctx))->txFifo.writeEnd();
        ((RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>*)(ctx))->sendOrReceive();
    }
    static bool write(void *ctx, uint8_t b) {
        return ((RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>*)(ctx))->txFifo.write(b);
    }


public:
    RFM12(RFM12Band band): txFifo(&_txFifo), rxFifo(&_rxFifo, false) {
        enable(band);
    }

    Mode getMode() {
        return mode;
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
        mode = Mode::IDLE;

        sei();
        sendOrListen();
    }

    inline Reader in() {
        return rxFifo.in();
    }

    inline Writer out() {
        return txFifo.out();
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
            // TODO
            //RFM12::commandIdle();
            //RFM12::sendOrReceive();
        }
    };

    OOK ook() {
        return OOK();
    }


};

template <const SPIMaster &spi, ChunkedFifo &_txFifo, ChunkedFifo &_rxFifo, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin>
const Writer::VTable RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>::writerVTable = {
    &RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>::writeStart,
    &RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>::writeEnd,
    &RFM12<spi,_txFifo,_rxFifo,ss_pin_t,ss_pin,int_pin_t,int_pin>::write
};


#endif /* RFM12_HPP_ */
