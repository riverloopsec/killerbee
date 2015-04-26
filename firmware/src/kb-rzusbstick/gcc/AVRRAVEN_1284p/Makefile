###############################################################################
# Makefile for the project AVRRAVEN_1284p
###############################################################################

## General Flags
PROJECT = AVRRAVEN_1284p
MCU = atmega1284p
TARGET = AVRRAVEN_1284p.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DAVR -DAVRRAVEN_1284P -ffunction-sections -fdata-sections  -DAVRRAVEN_1284P_APP_REV_MIN=0  -DAVRRAVEN_1284P_APP_REV_MAJ=2           -DLARGE_MEMORY  -DF_CPU=4000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += --gc-sections -mrelax -Wl,-Map=AVRRAVEN_1284p.map
LDFLAGS += -Wl,-section-start=FW_REV_ADR_M1284P_APP=0x1dffe


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\dev\KillerBee-Firmware\gcc\..\include\arch\avr" -I"C:\dev\KillerBee-Firmware\gcc\..\include\ieee802_15_4" -I"C:\dev\KillerBee-Firmware\gcc\..\include\tat" -I"C:\dev\KillerBee-Firmware\gcc\..\include\vrt" -I"C:\dev\KillerBee-Firmware\gcc\..\board" -I"C:\dev\KillerBee-Firmware\gcc\..\zigbee" -I"C:\dev\KillerBee-Firmware\gcc\..\include\application" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\arch\avr\dev" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\ieee802_15_4" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\tat" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\vrt" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\zigbee" -I"C:\dev\KillerBee-Firmware\gcc\..\include" -I"C:\dev\KillerBee-Firmware\gcc\..\include\zigbee" -I"C:\dev\KillerBee-Firmware\gcc\..\include\application\avrraven\1284p" -I"C:\dev\KillerBee-Firmware\gcc\..\include\application\avrraven" -I"C:\dev\KillerBee-Firmware\gcc\..\include\arch" 

## Libraries
LIBS = -lc -lm 

## Objects that must be built in order to link
OBJECTS = isr1284p.o avrraven_1284p.o cmd_if_1284p.o sipc.o at24cxx.o eep.o rf230_avr.o vrt_timer_avr.o ieee802_15_4_start.o ieee802_15_4.o ieee802_15_4_associate.o ieee802_15_4_beacon.o ieee802_15_4_cmd.o ieee802_15_4_data.o ieee802_15_4_disassociate.o ieee802_15_4_orphan.o ieee802_15_4_pib.o ieee802_15_4_poll.o ieee802_15_4_reset.o ieee802_15_4_rx_enable.o ieee802_15_4_scan.o tat_rf230.o vrt_mem.o vrt_kernel.o zigbee_start_router.o zigbee.o zigbee_data.o zigbee_discovery.o zigbee_formation.o zigbee_join.o zigbee_leave.o zigbee_neighbor_table.o zigbee_nib.o zigbee_permit_joining.o zigbee_poll.o self_programming.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) AVRRAVEN_1284p.hex AVRRAVEN_1284p.eep AVRRAVEN_1284p.lss## Compile
isr1284p.o: ../../application/avrraven/1284p/isr1284p.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

avrraven_1284p.o: ../../application/avrraven/1284p/avrraven_1284p.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

cmd_if_1284p.o: ../../application/avrraven/1284p/cmd_if_1284p.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sipc.o: ../../arch/avr/dev/sipc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

at24cxx.o: ../../arch/avr/dev/at24cxx.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

eep.o: ../../arch/avr/dev/eep.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rf230_avr.o: ../../arch/avr/dev/rf230_avr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vrt_timer_avr.o: ../../arch/avr/vrt/vrt_timer_avr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_start.o: ../../ieee802_15_4/ieee802_15_4_start.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4.o: ../../ieee802_15_4/ieee802_15_4.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_associate.o: ../../ieee802_15_4/ieee802_15_4_associate.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_beacon.o: ../../ieee802_15_4/ieee802_15_4_beacon.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_cmd.o: ../../ieee802_15_4/ieee802_15_4_cmd.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_data.o: ../../ieee802_15_4/ieee802_15_4_data.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_disassociate.o: ../../ieee802_15_4/ieee802_15_4_disassociate.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_orphan.o: ../../ieee802_15_4/ieee802_15_4_orphan.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_pib.o: ../../ieee802_15_4/ieee802_15_4_pib.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_poll.o: ../../ieee802_15_4/ieee802_15_4_poll.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_reset.o: ../../ieee802_15_4/ieee802_15_4_reset.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_rx_enable.o: ../../ieee802_15_4/ieee802_15_4_rx_enable.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_scan.o: ../../ieee802_15_4/ieee802_15_4_scan.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

tat_rf230.o: ../../tat/tat_rf230.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vrt_mem.o: ../../vrt/vrt_mem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vrt_kernel.o: ../../vrt/vrt_kernel.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_start_router.o: ../../zigbee/zigbee_start_router.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee.o: ../../zigbee/zigbee.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_data.o: ../../zigbee/zigbee_data.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_discovery.o: ../../zigbee/zigbee_discovery.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_formation.o: ../../zigbee/zigbee_formation.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_join.o: ../../zigbee/zigbee_join.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_leave.o: ../../zigbee/zigbee_leave.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_neighbor_table.o: ../../zigbee/zigbee_neighbor_table.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_nib.o: ../../zigbee/zigbee_nib.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_permit_joining.o: ../../zigbee/zigbee_permit_joining.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

zigbee_poll.o: ../../zigbee/zigbee_poll.c
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

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) AVRRAVEN_1284p.elf dep/* AVRRAVEN_1284p.hex AVRRAVEN_1284p.eep AVRRAVEN_1284p.lss AVRRAVEN_1284p.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

