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

GOOGLETEST_ROOT=external/googletest/googletest

TEST_SOURCEDIR=tst
TEST_BUILDDIR=target/test
TEST_SOURCES=$(wildcard $(TEST_SOURCEDIR)/*.cpp)
TEST_OBJECTS=$(patsubst $(TEST_SOURCEDIR)/%.cpp,$(TEST_BUILDDIR)/%.o,$(TEST_SOURCES)) 
GOOGLETEST_OBJECTS=$(TEST_BUILDDIR)/gtest-all.o $(TEST_BUILDDIR)/gtest_main.o
TEST_MAIN_OBJECTS=$(patsubst $(SOURCEDIR)/%.cpp,$(TEST_BUILDDIR)/%.o,$(SOURCES))
TEST_CPPFLAGS=-D__AVR_ATmega328P__ -DF_CPU=16000000 -Iinc -Itst -Iapps -I$(GOOGLETEST_ROOT)/include -std=gnu++14 -O1
TEST_TARGET=target/test/AvrLib
TEST_LDFLAGS=
TEST_LDLIBS=-lpthread

## Which tests to run
TESTS=* 

all: $(BUILDDIR) $(TARGET)

## These targets don't have files named after them
.PHONY: all disassemble disasm eeprom size flash fuses test

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

##  To make .o files from .cpp files 
$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp Makefile
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MMD -c -o "$@" "$<"

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
	
-include $(BUILDDIR)/*.d

$(TEST_BUILDDIR):
	mkdir -p $(TEST_BUILDDIR)

$(TEST_OBJECTS): $(TEST_BUILDDIR)/%.o: $(TEST_SOURCEDIR)/%.cpp Makefile
	g++ $(TEST_CPPFLAGS) -MMD -c -o "$@" "$<"

$(GOOGLETEST_OBJECTS): $(TEST_BUILDDIR)/%.o: $(GOOGLETEST_ROOT)/src/%.cc Makefile
	g++ -I$(GOOGLETEST_ROOT) $(TEST_CPPFLAGS) -MMD -c -o "$@" "$<"

$(TEST_MAIN_OBJECTS): $(TEST_BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp Makefile
	g++ $(TEST_CPPFLAGS) -MMD -c -o "$@" "$<"

$(TEST_TARGET): $(TEST_OBJECTS) $(TEST_MAIN_OBJECTS) $(GOOGLETEST_OBJECTS)
	g++ $(TEST_LDFLAGS) $^ $(TEST_LDLIBS) -o $@

-include $(TEST_BUILDDIR)/*.d

test: $(TEST_BUILDDIR) $(TEST_TARGET) 
	$(TEST_TARGET) --gtest_filter=$(TESTS)
