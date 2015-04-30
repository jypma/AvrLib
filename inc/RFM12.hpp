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
#include "SerialTx.hpp"
#include "RFM12TxFifo.hpp"
#include "RFM12RxFifo.hpp"
#include "RFM12Status.hpp"
#include "RFM12Strength.hpp"
#include "FS20Packet.hpp"
#include "SerialTx.hpp"

enum class RFM12Band: uint8_t {
    _433MHz = 1, _868Mhz = 2, _915MHz = 3
};

template <typename spi_t, spi_t &spi,
          typename ss_pin_t, ss_pin_t &ss_pin,
          typename int_pin_t, int_pin_t &int_pin,
          typename comparator_t, comparator_t &comparator,
          bool checkCrc = true,
          int rxFifoSize = 32, int txFifoSize = 32>
class RFM12 {
    typedef RFM12<spi_t,spi,ss_pin_t, ss_pin,int_pin_t,int_pin, comparator_t, comparator, checkCrc, rxFifoSize, txFifoSize> This;

    static void command(uint16_t cmd) {
        ss_pin.setLow();
        spi.send(cmd >> 8);
        spi.send(cmd);
        ss_pin.setHigh();
    }

    static const Writer::VTable writerVTable;

public:
    enum class Mode { IDLE, LISTENING, RECEIVING, SENDING_FSK, SENDING_OOK };

    volatile Mode mode = Mode::IDLE;
    //RFM12Strength drssi;
    RFM12TxFifo<txFifoSize> txFifo;
    RFM12RxFifo<rxFifoSize, checkCrc> rxFifo;

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
    //volatile int8_t lastStrength = 0;
    volatile uint8_t recvCount = 0;
    volatile uint8_t underruns = 0;
    volatile uint8_t ooks = 0;
    volatile uint16_t ook_bits = 0;
    volatile uint8_t ook_done = 0;
    PinD7 ookPin;

    void commandIdle() {
        command(0x820D);  // RF_IDLE_MODE
        mode = Mode::IDLE;
    }

    void sendOrListen() { // rf12_recvStart
        AtomicScope _;

        if (mode == Mode::IDLE) {
            //command(0x94A0 | drssi.reset());
            rxFifo.writeAbort();
            if (txFifo.hasContent()) {
                if (txFifo.readStart()) {
                    mode = Mode::SENDING_FSK;
                    command(0x823D); // RF_XMITTER_ON
                    //command(0xB8AA);     // RF_TXREG_WRITE 0xAA (preamble)
                } else {
                    ooks++;
                    mode = Mode::SENDING_OOK;
                    int_pin.interruptOff();
                    ookPin.setHigh();
                    command(0x820D);  // RF_IDLE_MODE
                    ookTx.sendFromSource();
                }
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
            if (mode == Mode::SENDING_OOK) {
                // there shouldn't be any interrupts from RFM12 during OOK sending
            } else if (mode == Mode::SENDING_FSK) {
                // we are sending
                if (txFifo.hasReadAvailable()) {
                    uint8_t b;
                    txFifo.read(b);
                    command(0xB800 | b); // RF_TXREG_WRITE
                } else {
                    commandIdle();
                    command(0xB8AA);     // RF_TXREG_WRITE 0xAA (postfix)
                    sendOrListen();
                }
            } else {
                // we are receiving / listening
                //TODO consider moving the spi read block in here, to not have the double if.
                recvCount++;

                //command(0x94A0 | drssi.apply(status));
                //lastStrength = drssi.getStrength();

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

        //command(0xCA83); // FIFO8,2-SYNC,!ff,DR
        //command(0xCE01); // SYNC=2DD4；

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

    InterruptHandler handler = { this, &This::interrupt };

    static void writeStart(void *ctx) {
        ((This*)(ctx))->txFifo.writeStart();
    }
    static void writeEnd(void *ctx) {
        ((This*)(ctx))->txFifo.writeEnd();
        ((This*)(ctx))->sendOrListen();
    }
    static bool write(void *ctx, uint8_t b) {
        return ((This*)(ctx))->txFifo.write(b);
    }

    friend struct OOKTarget;
    struct OOKTarget {
        This *rfm12;

        void setHigh(bool high) {
            if (high) {
                rfm12->command(0x823D); // RF_XMITTER_ON
            } else {
                rfm12->command(0x820D);  // RF_IDLE_MODE
            }
        }
    };

    friend struct OOKSource;
    struct OOKSource: public ChunkPulseSource {
        This *rfm12;

        OOKSource(This *_rfm12, ChunkedFifo &_fifo): ChunkPulseSource(&_fifo), rfm12(_rfm12) {}

        Pulse getNextPulse() {
            rfm12->ook_bits++;
            Pulse result = ChunkPulseSource::getNextPulse();
            if (result.isEmpty()) {
                rfm12->ook_done++;
/*
                rfm12->command(0x0000); // initial SPI transfer added to avoid power-up problem
                rfm12->command(0x8205); // RF_SLEEP_MODE: DC (disable clk pin), enable lbd

                // wait until RFM12B is out of power-up reset, this takes several *seconds*
                while (int_pin.isLow()) {
                    rfm12->command(0x0000);
                }

                rfm12->command(0x80C7 | (static_cast<uint8_t>(2) << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
                rfm12->command(0xA640); // 868MHz
                rfm12->command(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
                rfm12->command(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
                rfm12->command(0xC2AC); // AL,!ml,DIG,DQD4

                rfm12->command(0xCA8B); // FIFO8,1-SYNC,!ff,DR
                rfm12->command(0xCE2D); // SYNC=2D；

                rfm12->command(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
                rfm12->command(0x9850); // !mp,90kHz,MAX OUT
                rfm12->command(0xCC77); // OB1，OB0, LPX,！ddy，DDIT，BW0
                rfm12->command(0xE000); // NOT USE
                rfm12->command(0xC800); // NOT USE
                rfm12->command(0xC049); // 1.66MHz,3.1V
*/
                //rfm12->command(0xB8AA); // RF_TXREG_WRITE in case we're still in OOK mode
                //while (int_pin.isLow()) {
                //    rfm12->command(0x0000);
                // }
                rfm12->commandIdle();
                int_pin.interruptOnLow();
                rfm12->sendOrListen();
                rfm12->ookPin.setLow();
            } else {
                // because of SPI delays, and the DELAY and TRANSMITTER_ON messages being processed by the RFM12
                // at different delays, we need to make an adjustment between the desired pulse lengths, and the actual
                // forwarded pulse lengths.
                using namespace TimeUnits;
                constexpr auto correction = (240_us).template toCounts<comparator_t>();
                if (result.isHigh()) {
                    result = Pulse(true, result.getDuration() + correction);
                } else {
                    result = Pulse(false, result.getDuration() - correction);
                }
            }
            return result;
        }
    };

    OOKTarget ookTarget = { this };
    OOKSource ookSource = { this, txFifo.getChunkedFifo() };
    CallbackPulseTx<comparator_t, OOKTarget, OOKSource> ookTx = pulseTx(comparator, ookTarget, ookSource);
    SerialConfig fs20SerialConfig = FS20Packet::serialConfig<comparator_t>();
public:
    RFM12(RFM12Band band) {
        ookPin.configureAsOutput();
        enable(band);
    }

    Mode getMode() {
        return mode;
    }

    inline Reader in() {
        return rxFifo.in();
    }

    inline Writer out() {
        return txFifo.out(nullptr);
    }

    inline void out_fs20(const FS20Packet &packet) {
        txFifo.out(&fs20SerialConfig) << packet;
    }

    inline bool hasContent() const {
        return rxFifo.hasContent();
    }

};

template <typename spi_t, spi_t &spi, typename ss_pin_t, ss_pin_t &ss_pin, typename int_pin_t, int_pin_t &int_pin,
          typename comparator_t, comparator_t &comparator,
          bool checkCrc, int rxFifoSize, int txFifoSize>
const Writer::VTable RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,comparator_t,comparator,checkCrc,rxFifoSize,txFifoSize>::writerVTable = {
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,comparator_t,comparator,checkCrc,rxFifoSize,txFifoSize>::writeStart,
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,comparator_t,comparator,checkCrc,rxFifoSize,txFifoSize>::writeEnd,
    &RFM12<spi_t, spi,ss_pin_t,ss_pin,int_pin_t,int_pin,comparator_t,comparator,checkCrc,rxFifoSize,txFifoSize>::write
};


#endif /* RFM12_HPP_ */
