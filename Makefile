MCU=atmega328p
F_CPU=16000000UL
CC=avr-gcc
CXX=avr-g++
AR=avr-ar
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
AVRSIZE=avr-size
AVRDUDE=avrdude
PROGRAMMER_TYPE=arduino
PROGRAMMER_ARGS=
LFUSE = 0x62
HFUSE = 0xdf
EFUSE = 0x00

SOURCEDIR=src
BUILDDIR=target/$(MCU)

TARGET=$(BUILDDIR)/libAvrLib.a
#TARGET = $(lastword $(subst /, ,$(CURDIR)))

SOURCES=$(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS=$(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

## Compilation options, type man avr-gcc if you're curious.
CPPFLAGS = -DF_CPU=$(F_CPU) -Iinc -std=gnu++14 -fno-use-cxa-atexit 
CFLAGS = -Os -g -Wall
## Use short (8-bit) data types 
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
## Splits up object files per function
CFLAGS += -ffunction-sections -fdata-sections 

LDFLAGS = -Wl,-Map,$(TARGET).map 
## Optional, but often ends up with smaller code
LDFLAGS += -Wl,--gc-sections 
## Relax shrinks code even more, but makes disassembly messy
## LDFLAGS += -Wl,--relax
## LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm  ## for floating-point printf
## LDFLAGS += -Wl,-u,vfprintf -lprintf_min      ## for smaller printf
TARGET_ARCH = -mmcu=$(MCU)

all: directories $(TARGET)

## These targets don't have files named after them
.PHONY: all directories disassemble disasm eeprom size flash fuses

directories: $(BUILDDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

##  To make .o files from .cpp files 
$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp Makefile
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MMD -c -o "$@" "$<"

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
	
-include $(BUILDDIR)/*.d

