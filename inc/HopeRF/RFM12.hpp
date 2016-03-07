/*
 * RFM12.hpp
 *
 *  Created on: Jan 17, 2015
 *      Author: jan
 */

#ifndef RFM12_HPP_
#define RFM12_HPP_

#include "Logging.hpp"
#include "HAL/Atmel/InterruptVectors.hpp"
#include "SPI.hpp"
#include "BitSet.hpp"
#include "Fifo.hpp"
#include "CRC.hpp"
#include "ChunkedFifo.hpp"
#include "Serial/SerialTx.hpp"
#include "HopeRF/JeeLibTxFifo.hpp"
#include "HopeRF/JeeLibRxFifo.hpp"
#include "HopeRF/RFM12Status.hpp"
#include "HopeRF/RFM12Strength.hpp"
#include "FS20/FS20Packet.hpp"
#include "Serial/SerialTx.hpp"
#include "Time/Units.hpp"

namespace HopeRF {

using namespace Time;

enum class RFM12Band: uint8_t {
    _433MHz = 1, _868Mhz = 2, _915MHz = 3
};

enum class RFM12Mode: uint8_t { IDLE, LISTENING, RECEIVING, SENDING_FSK, SENDING_OOK };

template <typename spi_t,
          typename ss_pin_t,
          typename int_pin_t,
          typename comparator_t,
          bool checkCrc,
          int rxFifoSize, int txFifoSize>
class RFM12:
    public Streams::ReadingDelegate<
        RFM12<spi_t, ss_pin_t, int_pin_t, comparator_t, checkCrc, rxFifoSize, txFifoSize>,
        JeeLibRxFifo<5, rxFifoSize, checkCrc>>
{
    typedef RFM12<spi_t, ss_pin_t, int_pin_t, comparator_t, checkCrc, rxFifoSize, txFifoSize> This;
    typedef Logging::Log<Loggers::RFM12> log;

public:
    typedef RFM12Mode Mode;

private:
    struct CB {
        static void onWriteEnd(This &rfm) {
            rfm.sendOrListen();
        }
    };

    volatile Mode mode = Mode::IDLE;
    JeeLibTxFifo<CB, This, 5, txFifoSize> txFifo;
    JeeLibRxFifo<5, rxFifoSize, checkCrc> rxFifo;
    spi_t *spi;
    ss_pin_t *ss_pin;
    int_pin_t *int_pin;
    comparator_t *comparator;

    void command(uint16_t cmd) {
        ss_pin->setLow();
        spi->send(cmd >> 8);
        spi->send(cmd);
        ss_pin->setHigh();
    }

    BitSet<RFM12Status> getStatus(uint8_t &in) {
        ss_pin->setLow();
        // status bits are transmitted by RFM12 when sending 0x0000.
        uint16_t statusBits = spi->transceive(0x00) << 8;
        statusBits |= spi->transceive(0x00);
        auto res = BitSet<RFM12Status>(statusBits);

        if (res[RFM12Status::READY_FOR_NEXT_BYTE] && (mode == Mode::RECEIVING || mode == Mode::LISTENING)) {
            // RFM12's FIFO has a byte for us
            // slow down to under 2.5 MHz
            spi->setClockPrescaler(SPI2MHz);
            in = spi->transceive(0x00);
            spi->setClockPrescaler(SPI8MHz);
        }

        ss_pin->setHigh();
        return res;
    }
    volatile uint8_t ints = 0;
    volatile uint8_t recvCount = 0;
    volatile uint8_t underruns = 0;
    volatile uint16_t pulses = 0;

    void idle() {
        log::debug("idle()");
        command(0x820D);  // RF_IDLE_MODE
        mode = Mode::IDLE;
    }

    void sendOrListen() { // rf12_recvStart
        AtomicScope _;

        if (mode == Mode::LISTENING && txFifo.hasContent()) {
            idle();
        }
        if (mode == Mode::IDLE) {
            rxFifo.writeAbort();
            if (txFifo.hasContent()) {
                if (txFifo.readStart()) {
                    log::debug("sendOrListen(): send FSK");
                    mode = Mode::SENDING_FSK;
                    command(0x823D); // RF_XMITTER_ON
                    int_pin->interruptOnLow();
                } else {
                    log::debug("sendOrListen(): send OOK");
                    mode = Mode::SENDING_OOK;
                    int_pin->interruptOff();
                    command(0x820D);  // RF_IDLE_MODE
                    ookTx.sendFromSource();
                }
            } else {
                log::debug("sendOrListen(): listen");
                mode = Mode::LISTENING;
                int_pin->interruptOnLow();
                command(0x82DD); // RF_RECEIVER_ON
            }
        }
    }

    void onInterrupt() {
        ints++;
        uint8_t in = 0;
        // The RFM12 will keep the INT line low until we read the status register.
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
                    idle();
                    command(0xB8AA);     // RF_TXREG_WRITE 0xAA (postfix)
                    sendOrListen();
                }
            } else {
                // we are receiving / listening
                //TODO consider moving the spi read block in here, to not have the double if.

                if (rxFifo.isWriting()) {
                    rxFifo.write(in);
                } else {
                    rxFifo.writeStart(in);
                    mode = Mode::RECEIVING;
                }

                if (!rxFifo.isWriting()) {
                    // fifo expects no further bytes -> either we just received the last byte, or length was 0
                    recvCount++;
                    idle();
                    sendOrListen();
                }
            }

        }

        // power-on reset
        if (status[RFM12Status::POWER_ON_RESET]) {
            idle();
            sendOrListen();
        }

        // fifo overflow or buffer underrun - abort reception/sending
        if (status[RFM12Status::UNDERRUN_OVERFLOW]) {
            underruns++;
            rxFifo.writeAbort();
            txFifo.readAbort();
            idle();
            sendOrListen();
        }
    }

    void enable(RFM12Band band) {
        ss_pin->configureAsOutput();
        ss_pin->setHigh();
        int_pin->configureAsInputWithPullup();
        int_pin->interruptOnLow();

        cli();

        command(0x0000); // initial SPI transfer added to avoid power-up problem
        command(0x8205); // RF_SLEEP_MODE: DC (disable clk pin), enable lbd

        // wait until RFM12B is out of power-up reset, this takes several *seconds*
        command(0xB800); // RF_TXREG_WRITE in case we're still in OOK mode
        while (int_pin->isLow()) {
            command(0x0000);
        }

        command(0x80C7 | (static_cast<uint8_t>(band) << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
        command(0xA640); // 868MHz
        command(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
        command(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
        command(0xC2AC); // AL,!ml,DIG,DQD4

        //command(0xCA8B); // FIFO8,1-SYNC,!ff,DR
        //command(0xCE2D); // SYNC=2D；

        command(0xCA83); // FIFO8,2-SYNC,!ff,DR
        command(0xCE05); // SYNC=2D05；

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

    friend struct OOKTarget;
    struct OOKTarget {
        This *rfm12;

        void setHigh(bool high) const {
            if (rfm12->mode != Mode::SENDING_OOK) {
                return;
            }
            if (high) {
                rfm12->command(0x823D); // RF_XMITTER_ON
            } else {
                rfm12->command(0x820D);  // RF_IDLE_MODE
            }
        }

        void onInitialTransition(bool high) const {
            setHigh(high);
        }
        void onIntermediateTransition(bool high) const {
            setHigh(high);
        }
        void onFinalTransition(bool high) const {
            //rfm12->command(0x0000);
            //rfm12->command(0x8205); // RF_SLEEP_MODE: DC (disable clk pin), enable lbd
            rfm12->command(0xB800); // RF_TXREG_WRITE, we need to clear the TX buffer to leave OOK mode.
                                    // If we don't, we'll end in endless interrupt loop.
            while (rfm12->int_pin->isLow()) {
                rfm12->command(0x0000);
            }

            rfm12->idle();
            rfm12->sendOrListen();
        }
    };

    friend struct OOKSource;
    struct OOKSource: public ChunkPulseSource {
        This *rfm12;

        OOKSource(This *_rfm12, AbstractChunkedFifo &_fifo): ChunkPulseSource(_fifo), rfm12(_rfm12) {}

        Pulse getNextPulse() {
            rfm12->pulses++;
            Pulse result = ChunkPulseSource::getNextPulse();
            /*if (result.isEmpty()) {

                rfm12->command(0x0000);
                rfm12->command(0x8205); // RF_SLEEP_MODE: DC (disable clk pin), enable lbd
                rfm12->command(0xB800); // RF_TXREG_WRITE, we need to clear the TX buffer to leave OOK mode.
                                        // If we don't, we'll end in endless interrupt loop.
                while (rfm12->int_pin->isLow()) {
                    rfm12->command(0x0000);
                }

                //rfm12->idle();
                rfm12->sendOrListen();
            } else*/ if (!result.isEmpty()) {
                // because of SPI delays, and the DELAY and TRANSMITTER_ON messages being processed by the RFM12
                // at different delays, we need to make an adjustment between the desired pulse lengths, and the actual
                // forwarded pulse lengths.
                constexpr uint16_t correction = toCountsOn<comparator_t>(240_us);
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
    typedef PulseTx<comparator_t, OOKTarget, OOKSource> ookTx_t;
    ookTx_t ookTx = ookTx_t(*comparator, ookTarget, ookSource);
    SerialConfig fs20SerialConfig = FS20::FS20Packet::serialConfig<comparator_t>();

    void onComparator() {
        ookTx_t::onComparatorHandler::invoke(ookTx);
    }

public:
    RFM12(spi_t &_spi, ss_pin_t &_ss_pin, int_pin_t &_int_pin, comparator_t &_comparator, RFM12Band band):
        Streams::ReadingDelegate<
                RFM12<spi_t, ss_pin_t, int_pin_t, comparator_t, checkCrc, rxFifoSize, txFifoSize>,
                JeeLibRxFifo<5, rxFifoSize, checkCrc>>(&rxFifo),
        txFifo(*this), spi(&_spi), ss_pin(&_ss_pin), int_pin(&_int_pin), comparator(&_comparator) {
        enable(band);
    }

    void reset(RFM12Band band) {
        enable(band);
    }

    Mode getMode() const {
        return mode;
    }

    uint8_t getRecvCount() const {
        return recvCount;
    }

    uint8_t getUnderruns() const {
        return underruns;
    }

    uint8_t getInterrupts() const {
        return ints;
    }

    uint8_t getPulses() const {
        return pulses;
    }

    template <typename... types>
    bool write_fsk(uint8_t header, types... args) {
        return txFifo.write_fsk(header, args...);
    }

    bool write_fs20(const FS20::FS20Packet &packet) {
        log::debug("queueing FS20");
        return txFifo.write_ook(&fs20SerialConfig, &packet);
    }

    INTERRUPT_HANDLER1(typename int_pin_t::INT, onInterrupt);
    INTERRUPT_HANDLER2(typename comparator_t::INT, onComparator);
};

template <typename spi_t,
          typename ss_pin_t,
          typename int_pin_t,
          typename comparator_t,
          bool checkCrc = true,
          int rxFifoSize = 32, int txFifoSize = 32>
RFM12<spi_t, ss_pin_t, int_pin_t, comparator_t, checkCrc, rxFifoSize, txFifoSize> rfm12(spi_t &_spi, ss_pin_t &_ss_pin, int_pin_t &_int_pin, comparator_t &_comparator, RFM12Band band) {
    return RFM12<spi_t, ss_pin_t, int_pin_t, comparator_t, checkCrc, rxFifoSize, txFifoSize>(_spi, _ss_pin, _int_pin, _comparator, band);
}

}

#endif /* RFM12_HPP_ */
