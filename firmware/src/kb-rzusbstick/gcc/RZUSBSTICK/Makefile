###############################################################################
# Makefile for the project RZUSBSTICK
###############################################################################

# http://eleccelerator.com/fusecalc/fusecalc.php?chip=at90usb1287&HIGH=98&EXTENDED=FE&LOCKBIT=FF
# http://ww1.microchip.com/downloads/en/DeviceDoc/doc7618.pdf
# avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U hfuse:w:0x99:m


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
#LDFLAGS += --gc-sections -Wl,-Map=RZUSBSTICK.map
LDFLAGS += -Wl,--gc-sections -Wl,-Map=RZUSBSTICK.map 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"../../configure/arch/avr/dev" -I"../../configure" -I"../../configure/ieee802_15_4" -I"../../configure/tat" -I"../../configure/vrt" -I"../../configure/zigbee" -I"../../include" -I"../../include/application/rzusbstick" -I"../../include/arch" -I"../../include/arch/avr" -I"../../include/ieee802_15_4" -I"../../include/tat" -I"../../include/vrt" -I"../../include/zigbee" -I"../../board" 

## Libraries
LIBS = -lc -lm 

## Objects that must be built in order to link
OBJECTS = rzusbstick.o air_capture.o cmd_if.o isr.o usb_task.o at24cxx.o eep.o rf230_avr.o usb_descriptors.o usb_drv.o usb_specific_request.o usb_standard_request.o ieee802_15_4_pib.o ieee802_15_4_start.o ieee802_15_4.o ieee802_15_4_associate.o ieee802_15_4_beacon.o ieee802_15_4_cmd.o ieee802_15_4_data.o ieee802_15_4_disassociate.o ieee802_15_4_orphan.o ieee802_15_4_poll.o ieee802_15_4_reset.o ieee802_15_4_rx_enable.o ieee802_15_4_scan.o tat_rf230.o vrt_mem.o vrt_kernel.o zigbee_start_router.o zigbee.o zigbee_data.o zigbee_discovery.o zigbee_formation.o zigbee_join.o zigbee_leave.o zigbee_neighbor_table.o zigbee_nib.o zigbee_permit_joining.o zigbee_poll.o vrt_timer_avr.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build

all: $(TARGET) RZUSBSTICK.hex RZUSBSTICK.eep RZUSBSTICK.lss size

bootloader: ../RZUSBSTICK-BOOTLOADER/RZUSBSTICK_BOOTLOADER.hex
	@cd ../RZUSBSTICK-BOOTLOADER && make

## Install application and old bootloader using avrdude JTAG
install-old-bootloader:
	@srec_cat ../RZUSBSTICK-BOOTLOADER/RZUSBSTICK_BOOTLOADER.hex -Intel RZUSBSTICK.hex -Intel -o BOTH.hex -Intel
	@avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:BOTH.hex

# install using avrdude JTAG
install-with-avrdude:
	@avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:RZUSBSTICK.hex

# install using DFU with dfu-programmer
install-with-dfu-programmer:
	@kbbootloader
	@echo "Waiting 5 seconds for BOOTLOADER..."
	@sleep 5
	@dfu-programmer at90usb1287 erase --force
	@dfu-programmer at90usb1287 flash RZUSBSTICK.hex
	@echo
	@echo "Done! Now power cycle your RZUSBSTICK"
	@echo "Warning! If you have not set your fuses you will be stuck in DFU mode!"
	@echo "This can only be fixed with a hardware programmer such as AVR Dragon:"
	@echo "avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U hfuse:w:0x99:m"
	
# install using DFU with avrdude
install-with-dfu-avrdude:
	@kbbootloader
	@echo "Waiting 5 seconds for BOOTLOADER..."
	@sleep 5
	@avrdude -c flip1 -p usb1287 -U flash:w:RZUSBSTICK.hex -u
	@echo
	@echo "Done! Now power cycle your RZUSBSTICK"
	@echo "Warning! If you have not set your fuses you will be stuck in DFU mode!"
	@echo "This can only be fixed with a hardware programmer such as AVR Dragon:"
	@echo "avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U hfuse:w:0x99:m"
	
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
clean: mostlyclean
	-rm -rf RZUSBSTICK.hex

mostlyclean:
	-rm -rf $(OBJECTS) RZUSBSTICK.elf dep RZUSBSTICK.eep RZUSBSTICK.lss RZUSBSTICK.map BOTH.hex

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

