#ifndef TWI_HPP_
#define TWI_HPP_

#include "HAL/Atmel/InterruptHandlers.hpp"
#include "Fifo.hpp"
#include "ChunkedFifo.hpp"
#include "Logging.hpp"

namespace HAL {
namespace Atmel {
namespace Impl {

enum class TWIState { IDLE, WRITING, READING };

using namespace HAL::Atmel::InterruptHandlers;
using namespace HAL::Atmel::Registers;
using namespace Streams;

/**
 * Hardware Atmel TWI support. Based off the arduino libraries.
 *
 * TODO experiment with what happens when the bus is pulled low/high, e.g. with a floating
 * broken logic analyser. Interrupt won't occur in that case. Do we need a timeout?
 */
template <typename info_t, uint8_t txFifoSize, uint8_t rxFifoSize, uint32_t twiFreq>
class TWI {
	static constexpr uint8_t TW_START = 0x08;
	static constexpr uint8_t TW_REP_START = 0x10;
	static constexpr uint8_t TW_MT_SLA_ACK	= 0x18;
	static constexpr uint8_t TW_MT_SLA_NACK = 0x20;
	static constexpr uint8_t TW_MT_DATA_ACK = 0x28;
	static constexpr uint8_t TW_MT_DATA_NACK = 0x30;
	static constexpr uint8_t TW_MT_ARB_LOST= 0x38;
	static constexpr uint8_t TW_MR_SLA_ACK = 0x40;
	static constexpr uint8_t TW_MR_DATA_ACK= 0x50;
	static constexpr uint8_t TW_MR_DATA_NACK = 0x58;
	static constexpr uint8_t TW_MR_SLA_NACK = 0x48;
	static constexpr uint8_t TW_NO_INFO = 0xF8;
	static constexpr uint8_t TW_BUS_ERROR = 0x00;

	static constexpr uint8_t TW_WRITE = 0;
	static constexpr uint8_t TW_READ = 1;

	static uint8_t TW_STATUS() {
		return TWSR.get() & (TWS3 | TWS4 | TWS5 | TWS6 | TWS7);
	}

    typedef TWI<info_t, txFifoSize,rxFifoSize,twiFreq> This;
    typedef Logging::Log<Loggers::TWI> log;

    Fifo<txFifoSize> txFifoData;
    ChunkedFifo txFifo;// = { txFifoData };
    Fifo<rxFifoSize> rxFifoData;
    ChunkedFifo rxFifo;// = { rxFifoData };
    volatile uint8_t readExpected = 0;
public:
    static constexpr uint32_t frequency = twiFreq;

    volatile uint8_t ints = 0;

    static volatile bool transceiving;
    static void startWriting() {
    	log::debug(F("go!"), '0' + transceiving, ' ', dec(TWCR));
        if (!transceiving) {
            transceiving = true;
            TWCR = TWEN | TWIE | TWEA | TWINT | TWSTA;
        }
    }

    void replyAck() {
        TWCR = TWEN | TWIE | TWINT | TWEA;
    }

    void replyNack() {
        TWCR = TWEN | TWIE | TWINT;
    }

    void stop() {
        TWCR = TWEN | TWIE | TWEA | TWINT | TWSTO;

        // wait for stop condition to be executed on bus
        // TWINT is not set after a stop condition!
        uint16_t maxWait = 65000;
        while (maxWait > 0 && TWSTO.isSet()) maxWait--;

        txFifo.readEnd();
        rxFifo.writeEnd();
        readExpected = 0;
        transceiving = false;
        log::debug(F("stop"));
    }

    void releaseBus() {
        TWCR = TWEN | TWIE | TWEA | TWINT;
        txFifo.readEnd();
        rxFifo.writeEnd();
        readExpected = 0;
        transceiving = false;
        log::debug(F("rel"));
    }

    void onTWI() {
    	ints++;
    	//log::debug('t',':',dec(TW_STATUS));
        switch(TW_STATUS()) {
        // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
      // copy device address and r/w bit to output register and ack
    	uint8_t c;
    	c = txFifo.hasContent();
        txFifo.readStart();
    	uint8_t avail;
    	avail = txFifo.getReadAvailable();
        uint8_t address_and_rw_bit;
        txFifo.read(&address_and_rw_bit);
        log::debug('>',dec(address_and_rw_bit),',',dec(avail),',','0'+c);
        //log::debug(dec(uint16_t(&txFifo)));
        TWDR.set(address_and_rw_bit); // twi_slarw;
        replyAck();
      break;

    // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
        log::debug(F("TW_MT_SLA_ACK"));
        if (txFifo.hasReadAvailable()) {
            uint8_t b;
            txFifo.read(&b);
            TWDR.set(b);
            replyAck();
        } else {
            stop();
        }
      break;
    case TW_MT_DATA_ACK: // slave receiver acked data
        log::debug(F("TW_MT_DATA_ACK"));
        if (txFifo.hasReadAvailable()) {
            uint8_t b;
            txFifo.read(&b);
            TWDR.set(b);
            replyAck();
        } else {
            stop();
        }
      break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      log::debug(F("TW_MT_SLA_NACK"));
      stop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      log::debug(F("TW_MT_DATA_NACK"));
      stop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      log::debug(F("TW_MT_ARB_LOST"));
      releaseBus();
      break;

    // Master Receiver
    case TW_MR_DATA_ACK: // data received, ack sent
        log::debug(F("TW_MR_DATA_ACK "), dec(TWDR), dec(readExpected));
      // put byte into buffer
        rxFifo.write(TWDR);
        if (readExpected > 1) {
            readExpected--;
            replyAck();
        } else {
            readExpected = 0;
            replyNack();
        }
        break;
    case TW_MR_SLA_ACK:  // address sent, ack received
        log::debug(F("TW_MR_SLA_ACK "), dec(readExpected));
      // ack if more bytes are expected, otherwise nack
        if (readExpected > 0) {
            replyAck();
        } else {
            replyNack();
        }
        break;
    case TW_MR_DATA_NACK: // data received, nack sent
        log::debug(F("TW_MR_DATA_NACK "), dec(TWDR), dec(readExpected));
      // put final byte into buffer
        rxFifo.write(TWDR);
        stop();
        break;
    case TW_MR_SLA_NACK: // address sent, nack received
        log::debug(F("TW_MR_SLA_NACK"));
        stop();
      break;
    // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case


      /*
    // Slave Receiver
    case TW_SR_SLA_ACK:   // addressed, returned ack
    case TW_SR_GCALL_ACK: // addressed generally, returned ack
    case TW_SR_ARB_LOST_SLA_ACK:   // lost arbitration, returned ack
    case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
      // enter slave receiver mode
      twi_state = TWI_SRX;
      // indicate that rx buffer can be overwritten and ack
      twi_rxBufferIndex = 0;
      twi_reply(1);
      break;
    case TW_SR_DATA_ACK:       // data received, returned ack
    case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
      // if there is still room in the rx buffer
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        // put byte in buffer and ack
        twi_rxBuffer[twi_rxBufferIndex++] = TWDR;
        twi_reply(1);
      }else{
        // otherwise nack
        twi_reply(0);
      }
      break;
    case TW_SR_STOP: // stop or repeated start condition received
      // put a null char after data if there's room
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        twi_rxBuffer[twi_rxBufferIndex] = '\0';
      }
      // sends ack and stops interface for clock stretching
      twi_stop();
      // callback to user defined callback
      twi_onSlaveReceive(twi_rxBuffer, twi_rxBufferIndex);
      // since we submit rx buffer to "wire" library, we can reset it
      twi_rxBufferIndex = 0;
      // ack future responses and leave slave receiver state
      twi_releaseBus();
      break;
    case TW_SR_DATA_NACK:       // data received, returned nack
    case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
      // nack back at master
      twi_reply(0);
      break;

    // Slave Transmitter
    case TW_ST_SLA_ACK:          // addressed, returned ack
    case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
      // enter slave transmitter mode
      twi_state = TWI_STX;
      // ready the tx buffer index for iteration
      twi_txBufferIndex = 0;
      // set tx buffer length to be zero, to verify if user changes it
      twi_txBufferLength = 0;
      // request for txBuffer to be filled and length to be set
      // note: user must call twi_transmit(bytes, length) to do this
      twi_onSlaveTransmit();
      // if they didn't change buffer & length, initialize it
      if(0 == twi_txBufferLength){
        twi_txBufferLength = 1;
        twi_txBuffer[0] = 0x00;
      }
      // transmit first byte from buffer, fall
    case TW_ST_DATA_ACK: // byte sent, ack returned
      // copy data to output register
      TWDR = twi_txBuffer[twi_txBufferIndex++];
      // if there is more to send, ack, otherwise nack
      if(twi_txBufferIndex < twi_txBufferLength){
        twi_reply(1);
      }else{
        twi_reply(0);
      }
      break;
    case TW_ST_DATA_NACK: // received nack, we are done
    case TW_ST_LAST_DATA: // received ack, but we are done already!
      // ack future responses
      twi_reply(1);
      // leave slave receiver state
      twi_state = TWI_READY;
      break;
*/
    // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      log::debug(F("TW_BUS_ERROR"));
      stop();
      break;
        }
    }
public:
    typedef On<This, HAL::Atmel::Int_TWI_, &This::onTWI> Handlers;

    TWI(): txFifo(txFifoData), rxFifo(rxFifoData) {
    	//log::debug('s', dec(txFifo.getSize()), 'p', dec(txFifo.getSpace()));
        transceiving = false;

        // switch to input, without pull up for now
        info_t::PinSDA::DDR.clear();
        info_t::PinSDA::PORT.clear();
        info_t::PinSCL::DDR.clear();
        info_t::PinSCL::PORT.clear();

        // initialize twi prescaler and bit rate
        TWPS0.clear();
        TWPS1.clear();
        static_assert(((F_CPU / twiFreq) - 16) / 2 <= 255, "twiFreq is too low.");
        constexpr uint8_t twbr = ((F_CPU / twiFreq) - 16) / 2;
        static_assert(twbr >= 10, "TWBR should be 10 or higher for master mode.");
        TWBR.set(twbr);

          /* twi bit rate formula from atmega128 manual pg 204
          SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
          note: TWBR should be 10 or higher for master mode
          It is 72 for a 16mhz Wiring board with 100kHz TWI */

          // enable twi module, acks, and twi interrupt
        TWCR = TWEN | TWIE | TWEA;
    }

    bool isTransceiving() const {
    	return transceiving;
    }

    template <typename... types>
    bool write(uint8_t address, types... args) {
        return writeIfSpace(address, args...);
    }

    /* TODO re-enable this by switching away from ChunkedFifo, and having some other means of telling whether done.
     * This is because we can't read from a ChunkedFifo while also writing to it. So the TxFifo would need to become
     * an ordinary fifo. Perhaps combined with a "stillWriting" flag?
    template <typename... types>
    void writeOrBlock(uint8_t address, types... args) {
        txFifo.template writeOrBlockWith<&This::startWriting>(uint8_t(TW_WRITE | (address << 1)), args...);

        AtomicScope _;
        if (txFifo.hasContent()) {
            startWriting();
        }
    }
    */

    template <typename... types>
    bool writeIfSpace(uint8_t address, types... args) {
    	log::debug('s', dec(txFifo.getSize()), 'p', dec(txFifo.getSpace()), 'c', dec(txFifo.getCapacity()));
        bool result = txFifo.writeIfSpace(uint8_t(TW_WRITE | (address << 1)), args...);
        log::debug('w', dec(address), 'c', '0' + txFifo.hasContent(), 's', dec(txFifo.getSize()));
        //log::debug(dec(uint16_t(&txFifo)));

        txFifo.readStart();
    	uint8_t avail;
    	avail = txFifo.getReadAvailable();
        txFifo.readAbort();

        log::debug('a', dec(avail));

        AtomicScope _;
        if (txFifo.hasContent()) {
            startWriting();
        }

        return result;
    }

    void flush() {
    	if (SREG_I.isSet()) {
    		while (transceiving) ;
    	}
    }

    template <typename... types>
    ReadResult read(uint8_t address, types... args) {
        flush();
        readExpected = Streams::StreamedSize<types...>::fixedSizeReading;
        log::debug(F("read start: "), dec(readExpected));
        rxFifo.clear();
        rxFifo.writeStart();
        if (txFifo.write(uint8_t(TW_READ | (address << 1)))) {
        	startWriting();
        }
        flush();
        log::debug(F("read: "), dec(rxFifo.getSize()));
        return rxFifo.read(args...);
    }
};

template <typename info_t, uint8_t txFifoSize, uint8_t rxFifoSize, uint32_t twiFreq>
volatile bool TWI<info_t,txFifoSize,rxFifoSize,twiFreq>::transceiving = false;

}
}
}

#endif /* TWI_HPP_ */
