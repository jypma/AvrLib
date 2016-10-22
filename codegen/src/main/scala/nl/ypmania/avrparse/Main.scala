package nl.ypmania.avrparse

import scala.io.Source
import java.io.PrintWriter
import java.io.File

object Main extends App {
  // TODO StatusBitClearBy1
  // TODO other header files support, they're kinda messy
  // TODO xmega support (__AVR_ARCH__ >= 100)
  
  case class Register(name: String, 
      addr: Int,
      bit0: Seq[String] = Nil,
      bit1: Seq[String] = Nil,
      bit2: Seq[String] = Nil,
      bit3: Seq[String] = Nil,
      bit4: Seq[String] = Nil,
      bit5: Seq[String] = Nil,
      bit6: Seq[String] = Nil,
      bit7: Seq[String] = Nil) {
    def withBit(i: Int, name: String) = i match {
      case 0 => copy(bit0 = bit0 :+ name)
      case 1 => copy(bit1 = bit1 :+ name)
      case 2 => copy(bit2 = bit2 :+ name)
      case 3 => copy(bit3 = bit3 :+ name)
      case 4 => copy(bit4 = bit4 :+ name)
      case 5 => copy(bit5 = bit5 :+ name)
      case 6 => copy(bit6 = bit6 :+ name)
      case 7 => copy(bit7 = bit7 :+ name)
    }
    
    def print = {
        out.println(s"""using ${name}_t = Register8<0x${addr.toHexString},
          ${if (bit0.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit1.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit2.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit3.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit4.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit5.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit6.isEmpty) "ReservedBit" else "ReadWriteBit"},
          ${if (bit7.isEmpty) "ReservedBit" else "ReadWriteBit"}>;""")
          
        out.println(s"constexpr StaticRegister8<${name}_t> ${name} = {};")
        for (b <- bit0) { out.println(s"constexpr ${name}_t::Bit0 ${b} = {};") } 
        for (b <- bit1) { out.println(s"constexpr ${name}_t::Bit1 ${b} = {};") } 
        for (b <- bit2) { out.println(s"constexpr ${name}_t::Bit2 ${b} = {};") } 
        for (b <- bit3) { out.println(s"constexpr ${name}_t::Bit3 ${b} = {};") } 
        for (b <- bit4) { out.println(s"constexpr ${name}_t::Bit4 ${b} = {};") } 
        for (b <- bit5) { out.println(s"constexpr ${name}_t::Bit5 ${b} = {};") } 
        for (b <- bit6) { out.println(s"constexpr ${name}_t::Bit6 ${b} = {};") } 
        for (b <- bit7) { out.println(s"constexpr ${name}_t::Bit7 ${b} = {};") } 
        out.println()
    }
  }
  
  val io8start   = "#define ([A-Z0-9]+) +_SFR_IO8\\(0x([0-9A-F]+)\\)".r
  val mem8start  = "#define ([A-Z0-9]+) +_SFR_MEM8\\(0x([0-9A-F]+)\\)".r
  val mem16start = "#define ([A-Z0-9]+) +_SFR_MEM16\\(0x([0-9A-F]+)\\)".r
  val regbit = (0 to 7).map(i => s"#define ([A-Z0-9]+) $i".r).toSeq
  
  var currentRegister: Option[Register] = None
  
  val out = new PrintWriter(new File("ATMega328p_io.hpp"))
  out.println("""
#pragma once

#include "HAL/Register8.hpp"
#include "HAL/Register16.hpp"

namespace HAL {
namespace Atmel {
namespace Registers {
""")
  
  for (line <- Source.fromFile("avr/iom328p.h").getLines()) {
    for (m <- io8start.findFirstMatchIn(line)) {
      currentRegister.foreach(_.print)
        
      val name = m.group(1)
      val addr = Integer.parseInt(m.group(2), 16) + 0x20 // SFR_OFFSET for non-XMega
      currentRegister = Some(Register(name, addr))
    }
    
    for (m <- mem8start.findFirstMatchIn(line)) {
      currentRegister.foreach(_.print)
        
      val name = m.group(1)
      val addr = Integer.parseInt(m.group(2), 16)
      currentRegister = Some(Register(name, addr))
    }
    
    for (m <- mem16start.findFirstMatchIn(line)) {
      currentRegister.foreach(_.print)
      currentRegister = None // no individual bits in 16-bits registers
      
      val name = m.group(1)
      val addr = Integer.parseInt(m.group(2), 16)
      out.println(s"using ${name}_t = Register16<0x${addr.toHexString}>;")
      out.println(s"constexpr StaticRegister16<${name}_t> ${name} = {};")
      out.println()
    }
    
    for (i <- 0 to 7) {
      for (m <- regbit(i).findFirstMatchIn(line)) {
        val name = m.group(1)
        currentRegister = currentRegister.map(_.withBit(i, name))
      }      
    }
  }
  currentRegister.foreach(_.print) // print the last one
  
  out.println("""
} // namespace Registers
} // namespace Atmel
} // namespace HAL
""")

  out.close()
}