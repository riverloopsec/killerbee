Firmware
================

ApiMote
==================

The firmware for this is mainained and built using the GoodFET repository.

Compiled firmware versions are stored here for ease of use. However, you should often not need to use these as ApiMote v4betas provided by River Loop Security are all pre-flashed with the latest version.

To flash it, plug in the ApiMote v3 or v4beta and then run the `flash\_apimote.sh` script.
Note that it may sometimes take two attempts to flash correctly if the first time doesn't synchronize and times out.

RZUSBSTICK
==================

This needs to be flashed using external tools. Compiled firmware is:
- `kb-rzusbstick-001.hex`: Initial version
- `kb-rzusbstick-002.hex`: Newer version with EEMAGIC=0xFF, reported to be needed on newer models
- `kb-rzusbstick-003.hex`: Newer version built by Scytmo with fix for stalling on 64-byte packets

The source is included in `src/kb-rzusbstick/` folder. It should build on Windows and Ubuntu.

As described in the main `README.md`, you need to update the default firmware to support injection.
This process requires additional hardware and software and multiple methods are offered:

### OpenOCD and buspirate (Linux)

_Although we have not tested this, the submitter assures us it is working._

#### Pre-requisites

+ Hardware: Atmel RZ Raven USB Stick (RZUSBSTICK)
+ Hardware: Dangerous Prototypes Bus Pirate (http://dangerousprototypes.com/docs/Bus_Pirate)
+ Hardware: Six male-to-female Dupont wires
+ Software: OpenOCD (http://www.openocd.net/)
+ Software: KillerBee Firmware for the RZUSBSTICK

#### Procedure

1. Download OpenOCD and compile it with the support for the DP Bus Pirate.

```bash
sudo apt-get install git
git clone git://git.code.sf.net/p/openocd/code
sudo apt-get install libtool autoconf texinfo libusb-dev libftdi-dev pkg-config libusb-1.0.0
git clone git://git.code.sf.net/p/openocd/code
cd code
./bootstrap
./configure --enable-maintainer-mode --disable-werror --enable-buspirate
make
sudo make install
```

2. Obtain the KillerBee RZUSBSTICK firmware from `firmware/kb-rzusbstick-002.hex`.
Copy the firmware file into your Downloads directory.

3. Download the configuration file for OpenOCD and the DP buspirate form [here](https://gist.githubusercontent.com/mertenats/5150ce65a358cb91919fc3013ce81ab3/raw/3c17361366219f14805ac855540e30cfc4efac0e/openOCD_buspirate_rzusbstick.cfg) and edit the variable `_FIRMWARE_LOCATION`.

Example:
```
set  _FIRMWARE_LOCATION /home/user/repos/killerbee/firmware/kb-rzusbstick-002.hex
```

4. Connect the buspirate to the RZ Raven USB stick with the following connections: GND to GND (RZ Raven USB stick to buspirate), TCK to CLK, TDO to MISO, TMS to CS, TDI to MOSI and SRST to AUX (pins layouts available [here](http://esver.free.fr/upload/RZUSBstick-JTAG.png) and [here](http://esver.free.fr/upload/Bp-cable-color-hk.png)).

5. Execute the flashing process with OpenOCD.
```
openocd -f openocd_bp.cfg
```

References:
+ [JTAG debugging with Bus pirate and OpenOCD](https://research.kudelskisecurity.com/2014/05/01/jtag-debugging-made-easy-with-bus-pirate-and-openocd/)
+ [Utilisation d’OpenOCD pour programmer une clé RZUSBstick avec l’interface Bus Pirate](http://esver.free.fr/blog/?p=637)

### AVR Wireless Software (Windows)

_Although we have not tested this, jrussell88 developed this techniques and reports it as working._

#### Pre-requisites

+ Software: Windows 7 x86 (can't flash firmware with 64-bit drivers or OS) (tested in a VirtualBox VM under Ubuntu 17.04) 
+ [Atmel AVR Wireless Services AVR2017_RZRAVEN_Firmware.zip](http://palmtree.beeroclock.net/~karlp/AVR2017_RZRAVEN_Firmware.zip) where it is described as: "AVR2017: RZRAVEN Firmware (72585516, updated May 2008)"

#### Procedure

* Ensure Windows is up to date
* If in a VM such as VirtualBox: Plug in the RZUSBSTICK and add a USB filter in the VirtualBox Manager for the Windows VM by selecting RZUSBSTICK from the list under the VM's Settings > USB tab.
* In Windows, unzip `AVR2017_RZRAVEN_Firmware.zip` and install `AVRWirelessSetup.exe`.
* If this has installed correctly, Computer Management - Device Management should show an entry for Jungo, under which is Windriver whose properties show the `windrv6.sys` driver.
* The program should have installed to `C:\Program Files (x86)\Atmel\AVR Tools\AVR Wireless Services`.
* Replace the firmware `RFUSBSTICK-10_10.hex` in this folder with the new firmware (from KillerBee `firmware/`.
* Run Atmel - AVR Wireless Services from the Start Menu.
* From the Tools menu run "Upgrade usb stick"
* The dialog box offers IP address and port options which can be used to connect to a remote usb stick. However, the defaults should be fine.
* Click "Upgrade"
* It will generate an error message but should then say it has completed successfully
* The LED on the USB stick turns amber
* Firmware can be verified after installing KillerBee on Linux by running `sudo zbid`

### Atmel-ICE Basic (Windows)

_Although we have not tested this, the submitter assures us it is working._

#### Pre-requisities

+ Software: Windows
+ Software: AtmelStudio
+ Hardware: [Atmel-ICE Basic](http://www.atmel.com/tools/atatmel-ice.aspx) ([docs](http://www.atmel.com/Images/Atmel-42330-Atmel-ICE_UserGuide.pdf))

#### Procedure

* Install AtmelStudio and run it
* Connect Atmel-ICE Basic to USB port 
* Connect 10-pin connector to RZUSBstick and the other end to the AVR slot
* Connect RZUSBstick to the USB port
* In AtmelStudio go to Tools > Device Programming
* Choose these options:
  * Tools: Atmel-ICE
  * Device: AT90USB1287
  * Interface: JTAG
* Click "Apply"
* In the Device Signature press Read. If an error appears, you need to turn the 10 pin connector in the RZUSBstick the other way.
* Select "Memories"
* In the flash menu click "Read" and save a backup of the firmware
* Select the file with the new firmware (from KillerBee `firmware/`) and select "Erase device before programming" and "Verify Flash after programming"
* Press Program and this should appear in the status box:
```
Erasing device... OK
Programming Flash...OK
Verifying Flash...OK
```

### AVRDUDE and Atmel AVR Dragon (Windows / Linux)

#### Pre-requisites

+ Hardware: Atmel RZ Raven USB Stick (RZUSBSTICK)
+ Hardware: Atmel AVR Dragon On-Chip Programmer (ATAVRDRAGON)
+ Hardware: Atmel 100-mm to 50-mm JTAG Standoff Adapter (ATAVR-SOAKIT)
+ Hardware: 50mm male-to-male header (Digi-Key part S9015E-05)
+ Hardware: 10-pin (2x5) 100-mm female-to-female ribbon cable (Digi-Key part H3AAH-1018G-ND)
+ Software: AVRDUDE (http://winavr.sourceforge.net for Windows or http://www.nongnu.org/avrdude for Linux)
+ Software: KillerBee Firmware for the RZUSBSTICK

#### Procedure

* For Windows users, install the AVR Dragon drivers provided with [the libusb-win32 software](http://sourceforge.net/projects/libusb-win32).
  * Download and extract the zip file, then launch the
libusb-win32 `inf-wizard.exe` executable.
  * Connect the AVR Dragon to a USB port and click Next in the wizard to detect and identify the USB vendor ID and product ID 0x03EB and 0x2107 for the AVRDRAGON.
  * Complete the wizard by clicking Next, then Finish to install the drivers.
  * When prompted by Windows, click "Install This Driver Sofware Anyway".

* Copy the RZUSBSTICK firmware from `firmware/kb-rzusbstick-002.hex` to the directory where you extracted the AVRDUDE software.
  * Note: We are suggesting the `-002` version now as some people report that `-001` does not work on newer RZUSBSTICK versions. Change the filenames in the example commands below.

* Connect the AVR Dragon programmer to the ribbon cable, and connect the 100-mm to 50-mm adapter with the header.
* Prepare your terminal to flash the RZUSBSTICK by entering the following command at a
command prompt (but _do not_ hit enter yet):
```
avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:kb-rzusbstick-002.hex
```
  * On "older" RZUSBSTICKs, use the file `kb-rzusbstick-001.hex` instead.

* Insert the header into the RZUSBSTICK with pin 1 closest to the LEDs (farthest from the USB
connector).
  * You could solder it in place, but we don't bother. Just hold it at an angle so all the pins
make contact.
* Hit enter where you typed the AVRDUDE comand. You should see output similar to the following:

```bat
C:\avrdude>avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:kb-rzusbstick-002.hex

avrdude: jtagmkII_initialize(): warning: OCDEN fuse not programmed, single-byte EEPROM updates not possible
avrdude: AVR device initialized and ready to accept instructions
Reading | #################################################| 100% 0.05s
avrdude: Device signature = 0x1e9782
avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: jtagmkII_initialize(): warning: OCDEN fuse not programmed, single-byte EEPROM updates not possible
avrdude: reading input file "kb-rzusbstick-001.hex"
avrdude: input file kb-rzusbstick-001.hex auto detected as Intel Hex
avrdude: writing flash (26778 bytes):
Writing | #################################################| 100% 3.44s
avrdude: 26778 bytes of flash written
avrdude: verifying flash memory against kb-rzusbstick-001.hex:
avrdude: load data flash data from input file kb-rzusbstick-001.hex:
avrdude: input file kb-rzusbstick-001.hex auto detected as Intel Hex
avrdude: input file kb-rzusbstick-001.hex contains 26778 bytes
avrdude: reading on-chip flash data:
Reading | #################################################| 100% 3.79s
avrdude: verifying ...
avrdude: 26778 bytes of flash verified
avrdude: safemode: Fuses OK
avrdude done.  Thank you.
```

It should only take a few seconds to complete.
For a more detailed, picture-rich set of instructions, grab a copy of Hacking Exposed Wireless 3rd Edition (chapter 13).
