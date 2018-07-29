###############################################################################
# Makefile for the project RZUSBSTICK-BOOTLOADER
###############################################################################

## General Flags
PROJECT = RZUSBSTICK_BOOTLOADER
MCU = at90usb1287
TARGET = RZUSBSTICK_BOOTLOADER.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)

CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DAVR -DLITTLE_ENDIAN -DRZUSBSTICK -ffunction-sections  -fdata-sections  -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

## Version
##
## history - MAJ=0 = Original ATMEL code
##           MAJ=1 = KILLERBEE updates
##                   MIN=0 - Enter bootloader if no user application exists (freshly erased)
##                   MIN=1 - Set ERROR LED to BLUE
CFLAGS += -DBOOT_VER_MAJ=1 -DBOOT_VER_MIN=1

CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
#LDFLAGS += --gc-sections -Wl,-Map=RZUSBSTICK.map
LDFLAGS += -Wl,--gc-sections -Wl,--section-start=.text=1E000 -Wl,-Map=RZUSBSTICK_BOOTLOADER.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"../../configure/arch/avr" -I"../../configure/arch/avr/dev" -I"../../configure/vrt" -I"../../include" -I"../../include/application/rzusbstick" -I"../../include/arch/avr" -I"../../include/vrt" -I"../../board" 

## Libraries
LIBS = -lc -lm 

## Objects that must be built in order to link
OBJECTS = cmd_if_bootloader.o rzraven_bootloader.o vrt_mem.o vrt_kernel.o usb_standard_request.o usb_task.o at24cxx.o eep.o usb_descriptors.o usb_drv.o usb_specific_request.o self_programming.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) RZUSBSTICK_BOOTLOADER.hex RZUSBSTICK_BOOTLOADER.eep RZUSBSTICK_BOOTLOADER.lss size

## Install
install:
	@avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:RZUSBSTICK_BOOTLOADER.hex

## Compile
rzraven_bootloader.o: ../../application/rzusbstick/rzraven_bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

cmd_if_bootloader.o: ../../application/rzusbstick/cmd_if_bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_task.o: ../../arch/avr/dev/usb_task.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

at24cxx.o: ../../arch/avr/dev/at24cxx.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

eep.o: ../../arch/avr/dev/eep.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_descriptors.o: ../../arch/avr/dev/usb_descriptors.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_drv.o: ../../arch/avr/dev/usb_drv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_specific_request.o: ../../arch/avr/dev/usb_specific_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_standard_request.o: ../../arch/avr/dev/usb_standard_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vrt_mem.o: ../../vrt/vrt_mem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vrt_kernel.o: ../../vrt/vrt_kernel.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

self_programming.o: ../../arch/avr/bootloader/self_programming.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean: mostlyclean
	-rm -rf RZUSBSTICK_BOOTLOADER.hex

mostlyclean:
	-rm -rf $(OBJECTS) RZUSBSTICK_BOOTLOADER.elf dep RZUSBSTICK_BOOTLOADER.eep RZUSBSTICK_BOOTLOADER.lss RZUSBSTICK_BOOTLOADER.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

