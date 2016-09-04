/*
 * common.h
 *
 *  Created on: Jan 6, 2015
 *      Author: jan
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include "Streams/FixedSizes.hpp"
#include "Streams/ReadResult.hpp"
#include "Streams/Format.hpp"
#include <functional>

#define _AVR_IO_H_
#define __AVR_ATmega328P__

class SpecialFunctionRegister {
	//static std::function<void()> mkCallback();
	uint8_t value = 0;
	std::function<void()> callback;
private:
	void assign(uint8_t v) {
		value = v;
		if (callback != nullptr) callback();
	}
public:
	SpecialFunctionRegister();
	void operator= (uint8_t v) { assign(v); }
	operator uint8_t() const { return value; }
	bool operator== (int v) const { return value == v; }
	void operator|= (int v) { assign(value | v); }
	void operator&= (int v) { assign(value & v); }
};

inline bool operator==(int v, const SpecialFunctionRegister r) { return v == uint8_t(r); }

extern SpecialFunctionRegister sfr_mem[256];

class SpecialFunctionRegister16 {
	static uint16_t lastAddr;
	SpecialFunctionRegister &a;
	SpecialFunctionRegister &b;
public:
	SpecialFunctionRegister16(): a(sfr_mem[lastAddr]), b(sfr_mem[lastAddr + 1]) {
		lastAddr++;
	}
	void operator= (uint16_t v) {
		a = ((uint8_t*)(&v))[0];
		b = ((uint8_t*)(&v))[1];
	}
	operator uint16_t() const {
		uint16_t result;
		((uint8_t*)(&result))[0] = a;
		((uint8_t*)(&result))[1] = b;
		return result;
	}
};

extern SpecialFunctionRegister16 sfr_mem16[255];

namespace Streams { namespace Impl {

template<> struct StreamedSizeReading<SpecialFunctionRegister*>: public FixedSize<1> {};
template<> struct StreamedSizeWriting<SpecialFunctionRegister>: public FixedSize<1> {};

template <typename sem, typename fifo_t>
void write1unchecked(fifo_t &fifo, const SpecialFunctionRegister value) {
	sem::write(fifo, uint8_t(value));
}

template <typename fifo_t>
ReadResult read1unchecked(fifo_t &fifo, SpecialFunctionRegister *v) {
	fifo.uncheckedRead(*v);
	return ReadResult::Valid;
}

template <typename sem, typename fifo_t>
bool _writeFunc(void *ctx, uint8_t value) {
    fifo_t &fifo = *((fifo_t*)ctx);

    if (sem::canWrite(fifo, 1)) {
        sem::write(fifo, value);
        return true;
    } else {
        return false;
    }
}

template <typename sem, typename fifo_t, typename int_t>
bool _write1decimalInt(fifo_t &fifo, const Decimal<int_t> value) {
    return Format::format(&(_writeFunc<sem,fifo_t>), &fifo, value);
}

template <typename sem, typename fifo_t>
bool write1(fifo_t &fifo, const Decimal<SpecialFunctionRegister> value) {
    return _write1decimalInt<sem>(fifo, Decimal<uint8_t>(uint8_t(value.value)));
}

}} // Streams::Impl


//extern uint8_t sfr_mem[256];

#define _SFR_IO8(addr) sfr_mem[addr +  + 0x20]
#define _SFR_MEM8(addr) sfr_mem[addr]
#define _SFR_MEM16(addr) sfr_mem16[addr]

#define F_CPU 16000000

#    define SREG _SFR_IO8(0x3F)

#  define SREG_C  (0)
#  define SREG_Z  (1)
#  define SREG_N  (2)
#  define SREG_V  (3)
#  define SREG_S  (4)
#  define SREG_H  (5)
#  define SREG_T  (6)
#  define SREG_I  (7)

#define _VECTOR(idx) vector_##idx
#define ISR(name) void name ()

#define _BV(bit) (1 << (bit))

void cli();
void sei();

#endif /* COMMON_H_ */
