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
#include "InterruptHandler.hpp"
#include "BitSet.hpp"
#include "Fifo.hpp"
#include "CRC.hpp"
#include "ChunkedFifo.hpp"
#include "RFM12TxFifo.hpp"
#include "RFM12RxFifo.hpp"
#include "RFM12Status.hpp"
#include "RFM12Strength.hpp"

enum class RFM12Band: uint8_t {
    _433MHz = 1, _868Mhz = 2, _915MHz = 3
};

template <typename spi_t, spi_t &spi, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin, int rxFifoSize = 32, int txFifoSize = 32>
class RFM12 {
    typedef RFM12<spi_t,spi,ss_pin_t, ss_pin,int_pin_t,int_pin, rxFifoSize, txFifoSize> This;

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

public:
    volatile Mode mode = Mode::IDLE;
    RFM12Strength drssi;
    RFM12TxFifo<txFifoSize> txFifo;
    RFM12RxFifo<rxFifoSize> rxFifo;

    BitSet<RFM12Status> getStatus(uint8_t &in) {
        ss_pin.setLow();
        // status bits are transmitted by RFM12 when sending 0x0000.
        uint16_t statusBits = spi.transceive(0x00) << 8;
        statusBits |= spi.transceive(0x00);
        auto res = BitSet<RFM12Status>(statusBits);

        if (res[RFM12Status::READY_FOR_NEXT_BYTE] && (mode == Mode::RECEIVING || mode == Mode::LISTENING)) {
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

    void commandIdle() {
        command(0x820D);  // RF_IDLE_MODE
        mode = Mode::IDLE;
    }

    void sendOrListen() { // rf12_recvStart
        AtomicScope _;

        if (mode == Mode::IDLE) {
            command(0x94A0 | drssi.reset());
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
        if (status[RFM12Status::READY_FOR_NEXT_BYTE]) {
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

                command(0x94A0 | drssi.apply(status));
                lastStrength = drssi.getStrength();

                if (rxFifo.isWriting()) {
                    rxFifo.write(in);
                } else {
                    lastLen = in;
                    rxFifo.writeStart(in);
                    mode = Mode::RECEIVING;
                }

                if (!rxFifo.isWriting()) {
                    // fifo expects no further bytes -> either we just received the last byte, or length was 0
                    commandIdle();
                    sendOrListen();
                }
            }
        }

        // power-on reset
        if (status[RFM12Status::POWER_ON_RESET]) {
            commandIdle();
            sendOrListen();
        }

        // fifo overflow or buffer underrun - abort reception/sending
        if (status[RFM12Status::UNDERRUN_OVERFLOW]) {
            underruns++;
            rxFifo.writeAbort();
            txFifo.readAbort();
            commandIdle();
            sendOrListen();
        }

    }

    InterruptHandler handler = { this, &This::interrupt };

    static void writeStart(void *ctx) {
        ((This*)(ctx))->txFifo.writeStart();
    }
    static void writeEnd(void *ctx) {
        ((This*)(ctx))->txFifo.writeEnd();
        ((This*)(ctx))->sendOrReceive();
    }
    static bool write(void *ctx, uint8_t b) {
        return ((This*)(ctx))->txFifo.write(b);
    }


public:
    RFM12(RFM12Band band) {
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
        int_pin.interrupt().attach(handler);
        int_pin.interruptOnLow();

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

template <typename spi_t, spi_t &spi, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin, int rxFifoSize, int txFifoSize>
const Writer::VTable RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,rxFifoSize,txFifoSize>::writerVTable = {
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,rxFifoSize,txFifoSize>::writeStart,
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,rxFifoSize,txFifoSize>::writeEnd,
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,rxFifoSize,txFifoSize>::write
};


#endif /* RFM12_HPP_ */
