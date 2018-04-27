
#pragma once

#include "HAL/Register8.hpp"
#include "HAL/Register16.hpp"

namespace HAL {
namespace Atmel {
namespace Registers {

using PINB_t = Register8<0x23,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PINB_t> PINB = {};
constexpr PINB_t::Bit0 PINB0 = {};
constexpr PINB_t::Bit1 PINB1 = {};
constexpr PINB_t::Bit2 PINB2 = {};
constexpr PINB_t::Bit3 PINB3 = {};
constexpr PINB_t::Bit4 PINB4 = {};
constexpr PINB_t::Bit5 PINB5 = {};
constexpr PINB_t::Bit6 PINB6 = {};
constexpr PINB_t::Bit7 PINB7 = {};

using DDRB_t = Register8<0x24,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<DDRB_t> DDRB = {};
constexpr DDRB_t::Bit0 DDB0 = {};
constexpr DDRB_t::Bit1 DDB1 = {};
constexpr DDRB_t::Bit2 DDB2 = {};
constexpr DDRB_t::Bit3 DDB3 = {};
constexpr DDRB_t::Bit4 DDB4 = {};
constexpr DDRB_t::Bit5 DDB5 = {};
constexpr DDRB_t::Bit6 DDB6 = {};
constexpr DDRB_t::Bit7 DDB7 = {};

using PORTB_t = Register8<0x25,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PORTB_t> PORTB = {};
constexpr PORTB_t::Bit0 PORTB0 = {};
constexpr PORTB_t::Bit1 PORTB1 = {};
constexpr PORTB_t::Bit2 PORTB2 = {};
constexpr PORTB_t::Bit3 PORTB3 = {};
constexpr PORTB_t::Bit4 PORTB4 = {};
constexpr PORTB_t::Bit5 PORTB5 = {};
constexpr PORTB_t::Bit6 PORTB6 = {};
constexpr PORTB_t::Bit7 PORTB7 = {};

using PINC_t = Register8<0x26,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<PINC_t> PINC = {};
constexpr PINC_t::Bit0 PINC0 = {};
constexpr PINC_t::Bit1 PINC1 = {};
constexpr PINC_t::Bit2 PINC2 = {};
constexpr PINC_t::Bit3 PINC3 = {};
constexpr PINC_t::Bit4 PINC4 = {};
constexpr PINC_t::Bit5 PINC5 = {};
constexpr PINC_t::Bit6 PINC6 = {};

using DDRC_t = Register8<0x27,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<DDRC_t> DDRC = {};
constexpr DDRC_t::Bit0 DDC0 = {};
constexpr DDRC_t::Bit1 DDC1 = {};
constexpr DDRC_t::Bit2 DDC2 = {};
constexpr DDRC_t::Bit3 DDC3 = {};
constexpr DDRC_t::Bit4 DDC4 = {};
constexpr DDRC_t::Bit5 DDC5 = {};
constexpr DDRC_t::Bit6 DDC6 = {};

using PORTC_t = Register8<0x28,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<PORTC_t> PORTC = {};
constexpr PORTC_t::Bit0 PORTC0 = {};
constexpr PORTC_t::Bit1 PORTC1 = {};
constexpr PORTC_t::Bit2 PORTC2 = {};
constexpr PORTC_t::Bit3 PORTC3 = {};
constexpr PORTC_t::Bit4 PORTC4 = {};
constexpr PORTC_t::Bit5 PORTC5 = {};
constexpr PORTC_t::Bit6 PORTC6 = {};

using PIND_t = Register8<0x29,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PIND_t> PIND = {};
constexpr PIND_t::Bit0 PIND0 = {};
constexpr PIND_t::Bit1 PIND1 = {};
constexpr PIND_t::Bit2 PIND2 = {};
constexpr PIND_t::Bit3 PIND3 = {};
constexpr PIND_t::Bit4 PIND4 = {};
constexpr PIND_t::Bit5 PIND5 = {};
constexpr PIND_t::Bit6 PIND6 = {};
constexpr PIND_t::Bit7 PIND7 = {};

using DDRD_t = Register8<0x2a,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<DDRD_t> DDRD = {};
constexpr DDRD_t::Bit0 DDD0 = {};
constexpr DDRD_t::Bit1 DDD1 = {};
constexpr DDRD_t::Bit2 DDD2 = {};
constexpr DDRD_t::Bit3 DDD3 = {};
constexpr DDRD_t::Bit4 DDD4 = {};
constexpr DDRD_t::Bit5 DDD5 = {};
constexpr DDRD_t::Bit6 DDD6 = {};
constexpr DDRD_t::Bit7 DDD7 = {};

using PORTD_t = Register8<0x2b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PORTD_t> PORTD = {};
constexpr PORTD_t::Bit0 PORTD0 = {};
constexpr PORTD_t::Bit1 PORTD1 = {};
constexpr PORTD_t::Bit2 PORTD2 = {};
constexpr PORTD_t::Bit3 PORTD3 = {};
constexpr PORTD_t::Bit4 PORTD4 = {};
constexpr PORTD_t::Bit5 PORTD5 = {};
constexpr PORTD_t::Bit6 PORTD6 = {};
constexpr PORTD_t::Bit7 PORTD7 = {};

using TIFR0_t = Register8<0x35,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIFR0_t> TIFR0 = {};
constexpr TIFR0_t::Bit0 TOV0 = {};
constexpr TIFR0_t::Bit1 OCF0A = {};
constexpr TIFR0_t::Bit2 OCF0B = {};

using TIFR1_t = Register8<0x36,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIFR1_t> TIFR1 = {};
constexpr TIFR1_t::Bit0 TOV1 = {};
constexpr TIFR1_t::Bit1 OCF1A = {};
constexpr TIFR1_t::Bit2 OCF1B = {};
constexpr TIFR1_t::Bit5 ICF1 = {};

using TIFR2_t = Register8<0x37,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIFR2_t> TIFR2 = {};
constexpr TIFR2_t::Bit0 TOV2 = {};
constexpr TIFR2_t::Bit1 OCF2A = {};
constexpr TIFR2_t::Bit2 OCF2B = {};

using PCIFR_t = Register8<0x3b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<PCIFR_t> PCIFR = {};
constexpr PCIFR_t::Bit0 PCIF0 = {};
constexpr PCIFR_t::Bit1 PCIF1 = {};
constexpr PCIFR_t::Bit2 PCIF2 = {};

using EIFR_t = Register8<0x3c,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<EIFR_t> EIFR = {};
constexpr EIFR_t::Bit0 INTF0 = {};
constexpr EIFR_t::Bit1 INTF1 = {};

using EIMSK_t = Register8<0x3d,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<EIMSK_t> EIMSK = {};
constexpr EIMSK_t::Bit0 INT0 = {};
constexpr EIMSK_t::Bit1 INT1 = {};

using GPIOR0_t = Register8<0x3e,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<GPIOR0_t> GPIOR0 = {};
constexpr GPIOR0_t::Bit0 GPIOR00 = {};
constexpr GPIOR0_t::Bit1 GPIOR01 = {};
constexpr GPIOR0_t::Bit2 GPIOR02 = {};
constexpr GPIOR0_t::Bit3 GPIOR03 = {};
constexpr GPIOR0_t::Bit4 GPIOR04 = {};
constexpr GPIOR0_t::Bit5 GPIOR05 = {};
constexpr GPIOR0_t::Bit6 GPIOR06 = {};
constexpr GPIOR0_t::Bit7 GPIOR07 = {};

using EECR_t = Register8<0x3f,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<EECR_t> EECR = {};
constexpr EECR_t::Bit0 EERE = {};
constexpr EECR_t::Bit1 EEPE = {};
constexpr EECR_t::Bit2 EEMPE = {};
constexpr EECR_t::Bit3 EERIE = {};
constexpr EECR_t::Bit4 EEPM0 = {};
constexpr EECR_t::Bit5 EEPM1 = {};

using EEDR_t = Register8<0x40,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<EEDR_t> EEDR = {};
constexpr EEDR_t::Bit0 EEDR0 = {};
constexpr EEDR_t::Bit1 EEDR1 = {};
constexpr EEDR_t::Bit2 EEDR2 = {};
constexpr EEDR_t::Bit3 EEDR3 = {};
constexpr EEDR_t::Bit4 EEDR4 = {};
constexpr EEDR_t::Bit5 EEDR5 = {};
constexpr EEDR_t::Bit6 EEDR6 = {};
constexpr EEDR_t::Bit7 EEDR7 = {};

using EEARL_t = Register8<0x41,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<EEARL_t> EEARL = {};
constexpr EEARL_t::Bit0 EEAR0 = {};
constexpr EEARL_t::Bit1 EEAR1 = {};
constexpr EEARL_t::Bit2 EEAR2 = {};
constexpr EEARL_t::Bit3 EEAR3 = {};
constexpr EEARL_t::Bit4 EEAR4 = {};
constexpr EEARL_t::Bit5 EEAR5 = {};
constexpr EEARL_t::Bit6 EEAR6 = {};
constexpr EEARL_t::Bit7 EEAR7 = {};

using EEARH_t = Register8<0x42,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<EEARH_t> EEARH = {};
constexpr EEARH_t::Bit0 EEAR8 = {};
constexpr EEARH_t::Bit1 EEAR9 = {};

using GTCCR_t = Register8<0x43,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit>;
constexpr StaticRegister8<GTCCR_t> GTCCR = {};
constexpr GTCCR_t::Bit0 PSRSYNC = {};
constexpr GTCCR_t::Bit1 PSRASY = {};
constexpr GTCCR_t::Bit7 TSM = {};

using TCCR0A_t = Register8<0x44,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR0A_t> TCCR0A = {};
constexpr TCCR0A_t::Bit0 WGM00 = {};
constexpr TCCR0A_t::Bit1 WGM01 = {};
constexpr TCCR0A_t::Bit4 COM0B0 = {};
constexpr TCCR0A_t::Bit5 COM0B1 = {};
constexpr TCCR0A_t::Bit6 COM0A0 = {};
constexpr TCCR0A_t::Bit7 COM0A1 = {};

using TCCR0B_t = Register8<0x45,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR0B_t> TCCR0B = {};
constexpr TCCR0B_t::Bit0 CS00 = {};
constexpr TCCR0B_t::Bit1 CS01 = {};
constexpr TCCR0B_t::Bit2 CS02 = {};
constexpr TCCR0B_t::Bit3 WGM02 = {};
constexpr TCCR0B_t::Bit6 FOC0B = {};
constexpr TCCR0B_t::Bit7 FOC0A = {};

using TCNT0_t = Register8<0x46,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TCNT0_t> TCNT0 = {};

using OCR0A_t = Register8<0x47,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<OCR0A_t> OCR0A = {};

using OCR0B_t = Register8<0x48,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<OCR0B_t> OCR0B = {};

using GPIOR1_t = Register8<0x4a,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<GPIOR1_t> GPIOR1 = {};
constexpr GPIOR1_t::Bit0 GPIOR10 = {};
constexpr GPIOR1_t::Bit1 GPIOR11 = {};
constexpr GPIOR1_t::Bit2 GPIOR12 = {};
constexpr GPIOR1_t::Bit3 GPIOR13 = {};
constexpr GPIOR1_t::Bit4 GPIOR14 = {};
constexpr GPIOR1_t::Bit5 GPIOR15 = {};
constexpr GPIOR1_t::Bit6 GPIOR16 = {};
constexpr GPIOR1_t::Bit7 GPIOR17 = {};

using GPIOR2_t = Register8<0x4b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<GPIOR2_t> GPIOR2 = {};
constexpr GPIOR2_t::Bit0 GPIOR20 = {};
constexpr GPIOR2_t::Bit1 GPIOR21 = {};
constexpr GPIOR2_t::Bit2 GPIOR22 = {};
constexpr GPIOR2_t::Bit3 GPIOR23 = {};
constexpr GPIOR2_t::Bit4 GPIOR24 = {};
constexpr GPIOR2_t::Bit5 GPIOR25 = {};
constexpr GPIOR2_t::Bit6 GPIOR26 = {};
constexpr GPIOR2_t::Bit7 GPIOR27 = {};

using SPCR_t = Register8<0x4c,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<SPCR_t> SPCR = {};
constexpr SPCR_t::Bit0 SPR0 = {};
constexpr SPCR_t::Bit1 SPR1 = {};
constexpr SPCR_t::Bit2 CPHA = {};
constexpr SPCR_t::Bit3 CPOL = {};
constexpr SPCR_t::Bit4 MSTR = {};
constexpr SPCR_t::Bit5 DORD = {};
constexpr SPCR_t::Bit6 SPE = {};
constexpr SPCR_t::Bit7 SPIE = {};

using SPSR_t = Register8<0x4d,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<SPSR_t> SPSR = {};
constexpr SPSR_t::Bit0 SPI2X = {};
constexpr SPSR_t::Bit6 WCOL = {};
constexpr SPSR_t::Bit7 SPIF = {};

using SPDR_t = Register8<0x4e,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<SPDR_t> SPDR = {};
constexpr SPDR_t::Bit0 SPDR0 = {};
constexpr SPDR_t::Bit1 SPDR1 = {};
constexpr SPDR_t::Bit2 SPDR2 = {};
constexpr SPDR_t::Bit3 SPDR3 = {};
constexpr SPDR_t::Bit4 SPDR4 = {};
constexpr SPDR_t::Bit5 SPDR5 = {};
constexpr SPDR_t::Bit6 SPDR6 = {};
constexpr SPDR_t::Bit7 SPDR7 = {};

using ACSR_t = Register8<0x50,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ACSR_t> ACSR = {};
constexpr ACSR_t::Bit0 ACIS0 = {};
constexpr ACSR_t::Bit1 ACIS1 = {};
constexpr ACSR_t::Bit2 ACIC = {};
constexpr ACSR_t::Bit3 ACIE = {};
constexpr ACSR_t::Bit4 ACI = {};
constexpr ACSR_t::Bit5 ACO = {};
constexpr ACSR_t::Bit6 ACBG = {};
constexpr ACSR_t::Bit7 ACD = {};

using SMCR_t = Register8<0x53,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<SMCR_t> SMCR = {};
constexpr SMCR_t::Bit0 SE = {};
constexpr SMCR_t::Bit1 SM0 = {};
constexpr SMCR_t::Bit2 SM1 = {};
constexpr SMCR_t::Bit3 SM2 = {};

using MCUSR_t = Register8<0x54,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<MCUSR_t> MCUSR = {};
constexpr MCUSR_t::Bit0 PORF = {};
constexpr MCUSR_t::Bit1 EXTRF = {};
constexpr MCUSR_t::Bit2 BORF = {};
constexpr MCUSR_t::Bit3 WDRF = {};

using MCUCR_t = Register8<0x55,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<MCUCR_t> MCUCR = {};
constexpr MCUCR_t::Bit0 IVCE = {};
constexpr MCUCR_t::Bit1 IVSEL = {};
constexpr MCUCR_t::Bit4 PUD = {};
constexpr MCUCR_t::Bit5 BODSE = {};
constexpr MCUCR_t::Bit6 BODS = {};

using SPMCSR_t = Register8<0x57,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<SPMCSR_t> SPMCSR = {};
constexpr SPMCSR_t::Bit0 SELFPRGEN = {};
constexpr SPMCSR_t::Bit0 SPMEN = {};
constexpr SPMCSR_t::Bit1 PGERS = {};
constexpr SPMCSR_t::Bit2 PGWRT = {};
constexpr SPMCSR_t::Bit3 BLBSET = {};
constexpr SPMCSR_t::Bit4 RWWSRE = {};
constexpr SPMCSR_t::Bit5 SIGRD = {};
constexpr SPMCSR_t::Bit6 RWWSB = {};
constexpr SPMCSR_t::Bit7 SPMIE = {};

using WDTCSR_t = Register8<0x60,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<WDTCSR_t> WDTCSR = {};
constexpr WDTCSR_t::Bit0 WDP0 = {};
constexpr WDTCSR_t::Bit1 WDP1 = {};
constexpr WDTCSR_t::Bit2 WDP2 = {};
constexpr WDTCSR_t::Bit3 WDE = {};
constexpr WDTCSR_t::Bit4 WDCE = {};
constexpr WDTCSR_t::Bit5 WDP3 = {};
constexpr WDTCSR_t::Bit6 WDIE = {};
constexpr WDTCSR_t::Bit7 WDIF = {};

using CLKPR_t = Register8<0x61,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit>;
constexpr StaticRegister8<CLKPR_t> CLKPR = {};
constexpr CLKPR_t::Bit0 CLKPS0 = {};
constexpr CLKPR_t::Bit1 CLKPS1 = {};
constexpr CLKPR_t::Bit2 CLKPS2 = {};
constexpr CLKPR_t::Bit3 CLKPS3 = {};
constexpr CLKPR_t::Bit7 CLKPCE = {};

using PRR_t = Register8<0x64,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PRR_t> PRR = {};
constexpr PRR_t::Bit0 PRADC = {};
constexpr PRR_t::Bit1 PRUSART0 = {};
constexpr PRR_t::Bit2 PRSPI = {};
constexpr PRR_t::Bit3 PRTIM1 = {};
constexpr PRR_t::Bit5 PRTIM0 = {};
constexpr PRR_t::Bit6 PRTIM2 = {};
constexpr PRR_t::Bit7 PRTWI = {};

using OSCCAL_t = Register8<0x66,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<OSCCAL_t> OSCCAL = {};
constexpr OSCCAL_t::Bit0 CAL0 = {};
constexpr OSCCAL_t::Bit1 CAL1 = {};
constexpr OSCCAL_t::Bit2 CAL2 = {};
constexpr OSCCAL_t::Bit3 CAL3 = {};
constexpr OSCCAL_t::Bit4 CAL4 = {};
constexpr OSCCAL_t::Bit5 CAL5 = {};
constexpr OSCCAL_t::Bit6 CAL6 = {};
constexpr OSCCAL_t::Bit7 CAL7 = {};

using PCICR_t = Register8<0x68,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<PCICR_t> PCICR = {};
constexpr PCICR_t::Bit0 PCIE0 = {};
constexpr PCICR_t::Bit1 PCIE1 = {};
constexpr PCICR_t::Bit2 PCIE2 = {};

using EICRA_t = Register8<0x69,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<EICRA_t> EICRA = {};
constexpr EICRA_t::Bit0 ISC00 = {};
constexpr EICRA_t::Bit1 ISC01 = {};
constexpr EICRA_t::Bit2 ISC10 = {};
constexpr EICRA_t::Bit3 ISC11 = {};

using PCMSK0_t = Register8<0x6b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PCMSK0_t> PCMSK0 = {};
constexpr PCMSK0_t::Bit0 PCINT0 = {};
constexpr PCMSK0_t::Bit1 PCINT1 = {};
constexpr PCMSK0_t::Bit2 PCINT2 = {};
constexpr PCMSK0_t::Bit3 PCINT3 = {};
constexpr PCMSK0_t::Bit4 PCINT4 = {};
constexpr PCMSK0_t::Bit5 PCINT5 = {};
constexpr PCMSK0_t::Bit6 PCINT6 = {};
constexpr PCMSK0_t::Bit7 PCINT7 = {};

using PCMSK1_t = Register8<0x6c,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<PCMSK1_t> PCMSK1 = {};
constexpr PCMSK1_t::Bit0 PCINT8 = {};
constexpr PCMSK1_t::Bit1 PCINT9 = {};
constexpr PCMSK1_t::Bit2 PCINT10 = {};
constexpr PCMSK1_t::Bit3 PCINT11 = {};
constexpr PCMSK1_t::Bit4 PCINT12 = {};
constexpr PCMSK1_t::Bit5 PCINT13 = {};
constexpr PCMSK1_t::Bit6 PCINT14 = {};

using PCMSK2_t = Register8<0x6d,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<PCMSK2_t> PCMSK2 = {};
constexpr PCMSK2_t::Bit0 PCINT16 = {};
constexpr PCMSK2_t::Bit1 PCINT17 = {};
constexpr PCMSK2_t::Bit2 PCINT18 = {};
constexpr PCMSK2_t::Bit3 PCINT19 = {};
constexpr PCMSK2_t::Bit4 PCINT20 = {};
constexpr PCMSK2_t::Bit5 PCINT21 = {};
constexpr PCMSK2_t::Bit6 PCINT22 = {};
constexpr PCMSK2_t::Bit7 PCINT23 = {};

using TIMSK0_t = Register8<0x6e,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIMSK0_t> TIMSK0 = {};
constexpr TIMSK0_t::Bit0 TOIE0 = {};
constexpr TIMSK0_t::Bit1 OCIE0A = {};
constexpr TIMSK0_t::Bit2 OCIE0B = {};

using TIMSK1_t = Register8<0x6f,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIMSK1_t> TIMSK1 = {};
constexpr TIMSK1_t::Bit0 TOIE1 = {};
constexpr TIMSK1_t::Bit1 OCIE1A = {};
constexpr TIMSK1_t::Bit2 OCIE1B = {};
constexpr TIMSK1_t::Bit5 ICIE1 = {};

using TIMSK2_t = Register8<0x70,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TIMSK2_t> TIMSK2 = {};
constexpr TIMSK2_t::Bit0 TOIE2 = {};
constexpr TIMSK2_t::Bit1 OCIE2A = {};
constexpr TIMSK2_t::Bit2 OCIE2B = {};

using ADC_t = Register16<0x78>;
constexpr StaticRegister16<ADC_t> ADC = {};

using ADCW_t = Register16<0x78>;
constexpr StaticRegister16<ADCW_t> ADCW = {};

using ADCL_t = Register8<0x78,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ADCL_t> ADCL = {};
constexpr ADCL_t::Bit0 ADCL0 = {};
constexpr ADCL_t::Bit1 ADCL1 = {};
constexpr ADCL_t::Bit2 ADCL2 = {};
constexpr ADCL_t::Bit3 ADCL3 = {};
constexpr ADCL_t::Bit4 ADCL4 = {};
constexpr ADCL_t::Bit5 ADCL5 = {};
constexpr ADCL_t::Bit6 ADCL6 = {};
constexpr ADCL_t::Bit7 ADCL7 = {};

using ADCH_t = Register8<0x79,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ADCH_t> ADCH = {};
constexpr ADCH_t::Bit0 ADCH0 = {};
constexpr ADCH_t::Bit1 ADCH1 = {};
constexpr ADCH_t::Bit2 ADCH2 = {};
constexpr ADCH_t::Bit3 ADCH3 = {};
constexpr ADCH_t::Bit4 ADCH4 = {};
constexpr ADCH_t::Bit5 ADCH5 = {};
constexpr ADCH_t::Bit6 ADCH6 = {};
constexpr ADCH_t::Bit7 ADCH7 = {};

using ADCSRA_t = Register8<0x7a,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ADCSRA_t> ADCSRA = {};
constexpr ADCSRA_t::Bit0 ADPS0 = {};
constexpr ADCSRA_t::Bit1 ADPS1 = {};
constexpr ADCSRA_t::Bit2 ADPS2 = {};
constexpr ADCSRA_t::Bit3 ADIE = {};
constexpr ADCSRA_t::Bit4 ADIF = {};
constexpr ADCSRA_t::Bit5 ADATE = {};
constexpr ADCSRA_t::Bit6 ADSC = {};
constexpr ADCSRA_t::Bit7 ADEN = {};

using ADCSRB_t = Register8<0x7b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<ADCSRB_t> ADCSRB = {};
constexpr ADCSRB_t::Bit0 ADTS0 = {};
constexpr ADCSRB_t::Bit1 ADTS1 = {};
constexpr ADCSRB_t::Bit2 ADTS2 = {};
constexpr ADCSRB_t::Bit6 ACME = {};

using ADMUX_t = Register8<0x7c,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ADMUX_t> ADMUX = {};
constexpr ADMUX_t::Bit0 MUX0 = {};
constexpr ADMUX_t::Bit1 MUX1 = {};
constexpr ADMUX_t::Bit2 MUX2 = {};
constexpr ADMUX_t::Bit3 MUX3 = {};
constexpr ADMUX_t::Bit5 ADLAR = {};
constexpr ADMUX_t::Bit6 REFS0 = {};
constexpr ADMUX_t::Bit7 REFS1 = {};

using DIDR0_t = Register8<0x7e,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<DIDR0_t> DIDR0 = {};
constexpr DIDR0_t::Bit0 ADC0D = {};
constexpr DIDR0_t::Bit1 ADC1D = {};
constexpr DIDR0_t::Bit2 ADC2D = {};
constexpr DIDR0_t::Bit3 ADC3D = {};
constexpr DIDR0_t::Bit4 ADC4D = {};
constexpr DIDR0_t::Bit5 ADC5D = {};

using DIDR1_t = Register8<0x7f,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<DIDR1_t> DIDR1 = {};
constexpr DIDR1_t::Bit0 AIN0D = {};
constexpr DIDR1_t::Bit1 AIN1D = {};

using TCCR1A_t = Register8<0x80,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR1A_t> TCCR1A = {};
constexpr TCCR1A_t::Bit0 WGM10 = {};
constexpr TCCR1A_t::Bit1 WGM11 = {};
constexpr TCCR1A_t::Bit4 COM1B0 = {};
constexpr TCCR1A_t::Bit5 COM1B1 = {};
constexpr TCCR1A_t::Bit6 COM1A0 = {};
constexpr TCCR1A_t::Bit7 COM1A1 = {};

using TCCR1B_t = Register8<0x81,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR1B_t> TCCR1B = {};
constexpr TCCR1B_t::Bit0 CS10 = {};
constexpr TCCR1B_t::Bit1 CS11 = {};
constexpr TCCR1B_t::Bit2 CS12 = {};
constexpr TCCR1B_t::Bit3 WGM12 = {};
constexpr TCCR1B_t::Bit4 WGM13 = {};
constexpr TCCR1B_t::Bit6 ICES1 = {};
constexpr TCCR1B_t::Bit7 ICNC1 = {};

using TCCR1C_t = Register8<0x82,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR1C_t> TCCR1C = {};
constexpr TCCR1C_t::Bit6 FOC1B = {};
constexpr TCCR1C_t::Bit7 FOC1A = {};

using TCNT1_t = Register16<0x84>;
constexpr StaticRegister16<TCNT1_t> TCNT1 = {};

using TCNT1L_t = Register8<0x84,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCNT1L_t> TCNT1L = {};
constexpr TCNT1L_t::Bit0 TCNT1L0 = {};
constexpr TCNT1L_t::Bit1 TCNT1L1 = {};
constexpr TCNT1L_t::Bit2 TCNT1L2 = {};
constexpr TCNT1L_t::Bit3 TCNT1L3 = {};
constexpr TCNT1L_t::Bit4 TCNT1L4 = {};
constexpr TCNT1L_t::Bit5 TCNT1L5 = {};
constexpr TCNT1L_t::Bit6 TCNT1L6 = {};
constexpr TCNT1L_t::Bit7 TCNT1L7 = {};

using TCNT1H_t = Register8<0x85,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCNT1H_t> TCNT1H = {};
constexpr TCNT1H_t::Bit0 TCNT1H0 = {};
constexpr TCNT1H_t::Bit1 TCNT1H1 = {};
constexpr TCNT1H_t::Bit2 TCNT1H2 = {};
constexpr TCNT1H_t::Bit3 TCNT1H3 = {};
constexpr TCNT1H_t::Bit4 TCNT1H4 = {};
constexpr TCNT1H_t::Bit5 TCNT1H5 = {};
constexpr TCNT1H_t::Bit6 TCNT1H6 = {};
constexpr TCNT1H_t::Bit7 TCNT1H7 = {};

using ICR1_t = Register16<0x86>;
constexpr StaticRegister16<ICR1_t> ICR1 = {};

using ICR1L_t = Register8<0x86,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ICR1L_t> ICR1L = {};
constexpr ICR1L_t::Bit0 ICR1L0 = {};
constexpr ICR1L_t::Bit1 ICR1L1 = {};
constexpr ICR1L_t::Bit2 ICR1L2 = {};
constexpr ICR1L_t::Bit3 ICR1L3 = {};
constexpr ICR1L_t::Bit4 ICR1L4 = {};
constexpr ICR1L_t::Bit5 ICR1L5 = {};
constexpr ICR1L_t::Bit6 ICR1L6 = {};
constexpr ICR1L_t::Bit7 ICR1L7 = {};

using ICR1H_t = Register8<0x87,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<ICR1H_t> ICR1H = {};
constexpr ICR1H_t::Bit0 ICR1H0 = {};
constexpr ICR1H_t::Bit1 ICR1H1 = {};
constexpr ICR1H_t::Bit2 ICR1H2 = {};
constexpr ICR1H_t::Bit3 ICR1H3 = {};
constexpr ICR1H_t::Bit4 ICR1H4 = {};
constexpr ICR1H_t::Bit5 ICR1H5 = {};
constexpr ICR1H_t::Bit6 ICR1H6 = {};
constexpr ICR1H_t::Bit7 ICR1H7 = {};

using OCR1A_t = Register16<0x88>;
constexpr StaticRegister16<OCR1A_t> OCR1A = {};

using OCR1AL_t = Register8<0x88,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<OCR1AL_t> OCR1AL = {};
constexpr OCR1AL_t::Bit0 OCR1AL0 = {};
constexpr OCR1AL_t::Bit1 OCR1AL1 = {};
constexpr OCR1AL_t::Bit2 OCR1AL2 = {};
constexpr OCR1AL_t::Bit3 OCR1AL3 = {};
constexpr OCR1AL_t::Bit4 OCR1AL4 = {};
constexpr OCR1AL_t::Bit5 OCR1AL5 = {};
constexpr OCR1AL_t::Bit6 OCR1AL6 = {};
constexpr OCR1AL_t::Bit7 OCR1AL7 = {};

using OCR1AH_t = Register8<0x89,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<OCR1AH_t> OCR1AH = {};
constexpr OCR1AH_t::Bit0 OCR1AH0 = {};
constexpr OCR1AH_t::Bit1 OCR1AH1 = {};
constexpr OCR1AH_t::Bit2 OCR1AH2 = {};
constexpr OCR1AH_t::Bit3 OCR1AH3 = {};
constexpr OCR1AH_t::Bit4 OCR1AH4 = {};
constexpr OCR1AH_t::Bit5 OCR1AH5 = {};
constexpr OCR1AH_t::Bit6 OCR1AH6 = {};
constexpr OCR1AH_t::Bit7 OCR1AH7 = {};

using OCR1B_t = Register16<0x8a>;
constexpr StaticRegister16<OCR1B_t> OCR1B = {};

using OCR1BL_t = Register8<0x8a,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<OCR1BL_t> OCR1BL = {};
constexpr OCR1BL_t::Bit0 OCR1BL0 = {};
constexpr OCR1BL_t::Bit1 OCR1BL1 = {};
constexpr OCR1BL_t::Bit2 OCR1BL2 = {};
constexpr OCR1BL_t::Bit3 OCR1BL3 = {};
constexpr OCR1BL_t::Bit4 OCR1BL4 = {};
constexpr OCR1BL_t::Bit5 OCR1BL5 = {};
constexpr OCR1BL_t::Bit6 OCR1BL6 = {};
constexpr OCR1BL_t::Bit7 OCR1BL7 = {};

using OCR1BH_t = Register8<0x8b,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<OCR1BH_t> OCR1BH = {};
constexpr OCR1BH_t::Bit0 OCR1BH0 = {};
constexpr OCR1BH_t::Bit1 OCR1BH1 = {};
constexpr OCR1BH_t::Bit2 OCR1BH2 = {};
constexpr OCR1BH_t::Bit3 OCR1BH3 = {};
constexpr OCR1BH_t::Bit4 OCR1BH4 = {};
constexpr OCR1BH_t::Bit5 OCR1BH5 = {};
constexpr OCR1BH_t::Bit6 OCR1BH6 = {};
constexpr OCR1BH_t::Bit7 OCR1BH7 = {};

using TCCR2A_t = Register8<0xb0,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR2A_t> TCCR2A = {};
constexpr TCCR2A_t::Bit0 WGM20 = {};
constexpr TCCR2A_t::Bit1 WGM21 = {};
constexpr TCCR2A_t::Bit4 COM2B0 = {};
constexpr TCCR2A_t::Bit5 COM2B1 = {};
constexpr TCCR2A_t::Bit6 COM2A0 = {};
constexpr TCCR2A_t::Bit7 COM2A1 = {};

using TCCR2B_t = Register8<0xb1,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TCCR2B_t> TCCR2B = {};
constexpr TCCR2B_t::Bit0 CS20 = {};
constexpr TCCR2B_t::Bit1 CS21 = {};
constexpr TCCR2B_t::Bit2 CS22 = {};
constexpr TCCR2B_t::Bit3 WGM22 = {};
constexpr TCCR2B_t::Bit6 FOC2B = {};
constexpr TCCR2B_t::Bit7 FOC2A = {};

using TCNT2_t = Register8<0xb2,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<TCNT2_t> TCNT2 = {};

using OCR2A_t = Register8<0xb3,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<OCR2A_t> OCR2A = {};

using OCR2B_t = Register8<0xb4,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<OCR2B_t> OCR2B = {};

using ASSR_t = Register8<0xb6,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<ASSR_t> ASSR = {};
constexpr ASSR_t::Bit0 TCR2BUB = {};
constexpr ASSR_t::Bit1 TCR2AUB = {};
constexpr ASSR_t::Bit2 OCR2BUB = {};
constexpr ASSR_t::Bit3 OCR2AUB = {};
constexpr ASSR_t::Bit4 TCN2UB = {};
constexpr ASSR_t::Bit5 AS2 = {};
constexpr ASSR_t::Bit6 EXCLK = {};

using TWBR_t = Register8<0xb8,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TWBR_t> TWBR = {};
constexpr TWBR_t::Bit0 TWBR0 = {};
constexpr TWBR_t::Bit1 TWBR1 = {};
constexpr TWBR_t::Bit2 TWBR2 = {};
constexpr TWBR_t::Bit3 TWBR3 = {};
constexpr TWBR_t::Bit4 TWBR4 = {};
constexpr TWBR_t::Bit5 TWBR5 = {};
constexpr TWBR_t::Bit6 TWBR6 = {};
constexpr TWBR_t::Bit7 TWBR7 = {};

using TWSR_t = Register8<0xb9,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TWSR_t> TWSR = {};
constexpr TWSR_t::Bit0 TWPS0 = {};
constexpr TWSR_t::Bit1 TWPS1 = {};
constexpr TWSR_t::Bit3 TWS3 = {};
constexpr TWSR_t::Bit4 TWS4 = {};
constexpr TWSR_t::Bit5 TWS5 = {};
constexpr TWSR_t::Bit6 TWS6 = {};
constexpr TWSR_t::Bit7 TWS7 = {};

using TWAR_t = Register8<0xba,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TWAR_t> TWAR = {};
constexpr TWAR_t::Bit0 TWGCE = {};
constexpr TWAR_t::Bit1 TWA0 = {};
constexpr TWAR_t::Bit2 TWA1 = {};
constexpr TWAR_t::Bit3 TWA2 = {};
constexpr TWAR_t::Bit4 TWA3 = {};
constexpr TWAR_t::Bit5 TWA4 = {};
constexpr TWAR_t::Bit6 TWA5 = {};
constexpr TWAR_t::Bit7 TWA6 = {};

using TWDR_t = Register8<0xbb,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TWDR_t> TWDR = {};
constexpr TWDR_t::Bit0 TWD0 = {};
constexpr TWDR_t::Bit1 TWD1 = {};
constexpr TWDR_t::Bit2 TWD2 = {};
constexpr TWDR_t::Bit3 TWD3 = {};
constexpr TWDR_t::Bit4 TWD4 = {};
constexpr TWDR_t::Bit5 TWD5 = {};
constexpr TWDR_t::Bit6 TWD6 = {};
constexpr TWDR_t::Bit7 TWD7 = {};

using TWCR_t = Register8<0xbc,
          ReadWriteBit,
          ReservedBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<TWCR_t> TWCR = {};
constexpr TWCR_t::Bit0 TWIE = {};
constexpr TWCR_t::Bit2 TWEN = {};
constexpr TWCR_t::Bit3 TWWC = {};
constexpr TWCR_t::Bit4 TWSTO = {};
constexpr TWCR_t::Bit5 TWSTA = {};
constexpr TWCR_t::Bit6 TWEA = {};
constexpr TWCR_t::Bit7 TWINT = {};

using TWAMR_t = Register8<0xbd,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReservedBit>;
constexpr StaticRegister8<TWAMR_t> TWAMR = {};
constexpr TWAMR_t::Bit0 TWAM0 = {};
constexpr TWAMR_t::Bit1 TWAM1 = {};
constexpr TWAMR_t::Bit2 TWAM2 = {};
constexpr TWAMR_t::Bit3 TWAM3 = {};
constexpr TWAMR_t::Bit4 TWAM4 = {};
constexpr TWAMR_t::Bit5 TWAM5 = {};
constexpr TWAMR_t::Bit6 TWAM6 = {};

using UCSR0A_t = Register8<0xc0,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<UCSR0A_t> UCSR0A = {};
constexpr UCSR0A_t::Bit0 MPCM0 = {};
constexpr UCSR0A_t::Bit1 U2X0 = {};
constexpr UCSR0A_t::Bit2 UPE0 = {};
constexpr UCSR0A_t::Bit3 DOR0 = {};
constexpr UCSR0A_t::Bit4 FE0 = {};
constexpr UCSR0A_t::Bit5 UDRE0 = {};
constexpr UCSR0A_t::Bit6 TXC0 = {};
constexpr UCSR0A_t::Bit7 RXC0 = {};

using UCSR0B_t = Register8<0xc1,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<UCSR0B_t> UCSR0B = {};
constexpr UCSR0B_t::Bit0 TXB80 = {};
constexpr UCSR0B_t::Bit1 RXB80 = {};
constexpr UCSR0B_t::Bit2 UCSZ02 = {};
constexpr UCSR0B_t::Bit3 TXEN0 = {};
constexpr UCSR0B_t::Bit4 RXEN0 = {};
constexpr UCSR0B_t::Bit5 UDRIE0 = {};
constexpr UCSR0B_t::Bit6 TXCIE0 = {};
constexpr UCSR0B_t::Bit7 RXCIE0 = {};

using UCSR0C_t = Register8<0xc2,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit,
          ReadWriteBit>;
constexpr StaticRegister8<UCSR0C_t> UCSR0C = {};
constexpr UCSR0C_t::Bit0 UCPOL0 = {};
constexpr UCSR0C_t::Bit1 UCSZ00 = {};
constexpr UCSR0C_t::Bit1 UCPHA0 = {};
constexpr UCSR0C_t::Bit2 UCSZ01 = {};
constexpr UCSR0C_t::Bit2 UDORD0 = {};
constexpr UCSR0C_t::Bit3 USBS0 = {};
constexpr UCSR0C_t::Bit4 UPM00 = {};
constexpr UCSR0C_t::Bit5 UPM01 = {};
constexpr UCSR0C_t::Bit6 UMSEL00 = {};
constexpr UCSR0C_t::Bit7 UMSEL01 = {};

using UBRR0_t = Register16<0xc4>;
constexpr StaticRegister16<UBRR0_t> UBRR0 = {};

using UBRR0L_t = Register8<0xc4,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<UBRR0L_t> UBRR0L = {};

using UBRR0H_t = Register8<0xc5,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<UBRR0H_t> UBRR0H = {};

using UDR0_t = Register8<0xc6,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit,
          ReservedBit>;
constexpr StaticRegister8<UDR0_t> UDR0 = {};


} // namespace Registers
} // namespace Atmel
} // namespace HAL

