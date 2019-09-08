all:
	@echo "Use 'make install-with-avrdude' to install DFU bootloader to your RZUSBSTICK"
	@echo "This will completely wipe your stick and previous bootloader so use with caution!"

# install and set fuse so hardware boot vector is not forcing bootloader to run
install-with-avrdude:
	@avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:128X-usb-boot-1-0-1.hex -u
	@sleep 2
	@avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U hfuse:w:0x99:m
	@echo "Done! Your RZUSBSTICK should now be in DFU mode and available as"
	@echo "'AT90USB128 DFU' device on your USB bus (idVendor=03eb, idProduct=2ffb)."
	@echo "To re-enable DFU mode any time after installing application firmware"
	@echo "use the 'kbbootloader' utility."
