###############################################################################
# Makefile for the project RZUSBSTICK
###############################################################################

## General Flags
PROJECT = RZUSBSTICK
MCU = at90usb1287
TARGET = RZUSBSTICK.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DAVR    -DRZUSBSTICK  -DLITTLE_ENDIAN      -DRZRAVEN_FW_MAJ=1  -DRZRAVEN_FW_MIN=0     -ffunction-sections  -fdata-sections  -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += --gc-sections -Wl,-Map=RZUSBSTICK.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\dev\KillerBee-Firmware\gcc\..\configure\arch\avr\dev" -I"C:\dev\KillerBee-Firmware\gcc\..\configure" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\ieee802_15_4" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\tat" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\vrt" -I"C:\dev\KillerBee-Firmware\gcc\..\configure\zigbee" -I"C:\dev\KillerBee-Firmware\gcc\..\include" -I"C:\dev\KillerBee-Firmware\gcc\..\include\application\rzusbstick" -I"C:\dev\KillerBee-Firmware\gcc\..\include\arch" -I"C:\dev\KillerBee-Firmware\gcc\..\include\arch\avr" -I"C:\dev\KillerBee-Firmware\gcc\..\include\ieee802_15_4" -I"C:\dev\KillerBee-Firmware\gcc\..\include\tat" -I"C:\dev\KillerBee-Firmware\gcc\..\include\vrt" -I"C:\dev\KillerBee-Firmware\gcc\..\include\zigbee" -I"C:\dev\KillerBee-Firmware\gcc\..\board" 

## Libraries
LIBS = -lc -lm 

## Objects that must be built in order to link
OBJECTS = rzusbstick.o air_capture.o cmd_if.o isr.o usb_task.o at24cxx.o eep.o rf230_avr.o usb_descriptors.o usb_drv.o usb_specific_request.o usb_standard_request.o ieee802_15_4_pib.o ieee802_15_4_start.o ieee802_15_4.o ieee802_15_4_associate.o ieee802_15_4_beacon.o ieee802_15_4_cmd.o ieee802_15_4_data.o ieee802_15_4_disassociate.o ieee802_15_4_orphan.o ieee802_15_4_poll.o ieee802_15_4_reset.o ieee802_15_4_rx_enable.o ieee802_15_4_scan.o tat_rf230.o vrt_mem.o vrt_kernel.o zigbee_start_router.o zigbee.o zigbee_data.o zigbee_discovery.o zigbee_formation.o zigbee_join.o zigbee_leave.o zigbee_neighbor_table.o zigbee_nib.o zigbee_permit_joining.o zigbee_poll.o vrt_timer_avr.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) RZUSBSTICK.hex RZUSBSTICK.eep RZUSBSTICK.lss size

## Compile
rzusbstick.o: ../../application/rzusbstick/rzusbstick.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

air_capture.o: ../../application/rzusbstick/air_capture.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

cmd_if.o: ../../application/rzusbstick/cmd_if.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

isr.o: ../../application/rzusbstick/isr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_task.o: ../../arch/avr/dev/usb_task.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

at24cxx.o: ../../arch/avr/dev/at24cxx.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

eep.o: ../../arch/avr/dev/eep.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rf230_avr.o: ../../arch/avr/dev/rf230_avr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_descriptors.o: ../../arch/avr/dev/usb_descriptors.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_drv.o: ../../arch/avr/dev/usb_drv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_specific_request.o: ../../arch/avr/dev/usb_specific_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_standard_request.o: ../../arch/avr/dev/usb_standard_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ieee802_15_4_pib.o: ../../ieee802_15_4/ieee802_15_4_pib.c
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

vrt_timer_avr.o: ../../arch/avr/vrt/vrt_timer_avr.c
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
clean:
	-rm -rf $(OBJECTS) RZUSBSTICK.elf dep/* RZUSBSTICK.hex RZUSBSTICK.eep RZUSBSTICK.lss RZUSBSTICK.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

