KillerBee
================

This is KillerBee - Framework and Tools for Attacking ZigBee and IEEE 802.15.4
networks.


MAINTAINERS/LICENSE
================

Distributed under a BSD license, see LICENSE for details.
All Rights Reserved.

The main toolkit was/is authored by:
+ 2009, Joshua Wright <jwright@willhackforsushi.com>
+ 2010-2015, Ryan Speers <ryan@riverloopsecurity.com>
+ 2010-2011, Ricky Melgares <ricky@riverloopsecurity.com>

We appreciate the many contributers to the framework, including the following who have contributed
capabilities:
+ Anonymous Contributor
+ Spencer McIntyre (scapy extension)
+ Bryan Halfpap <Bryanhalf@gmail.com> (additional tools)


REQUIREMENTS
================
KillerBee is developed and tested on Linux systems.  Windows support may be
added in the future.

We have striven to use a minimum number of software dependencies, however, it
is necessary to install the following Python modules before installation:

+ serial
+ usb
+ crypto  (for some functions)
+ pygtk   (for use of tools that have GUIs)
+ cairo   (for use of tools that have GUIs)
+ scapy-com (for some tools which utilize 802.15.4 Scapy extensions)

On Ubuntu systems, you can install the needed dependencies with the following
commands:

```
# apt-get install python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev
# hg clone https://bitbucket.org/secdev/scapy-com
# cd scapy-com
# python setup.py install
```

The python-dev and libgcrypt are required for the Scapy Extension Patch.

Also note that this is a fairly advanced and un-friendly attack platform.  This
is not Cain & Abel.  It is intended for developers and advanced analysts who are
attacking ZigBee and IEEE 802.15.4 networks.  I recommend you gain some
understanding of the ZigBee protocol (the book ZigBee Wireless Networks and
Transceivers by Shahin Farahani at http://bit.ly/2I5ppI is reasonable, though
still not great) and familiarity with the Python language before digging into
this framework.


INSTALLATION
================
KillerBee uses the standard Python 'setup.py' installation file.  Install
KillerBee with the following command:

```
# python setup.py install
```

DIRECTORIES
================
The directory structure for the KillerBee code is described as follows:

+ doc       - HTML documentation on the KillerBee library, courtesy of epydoc.
+ firmware  - Firmware for supported KillerBee hardware devices.
+ killerbee - Python library source.
+ sample    - Sample packet captures, referenced below.
+ scripts   - Shell scripts used in development.
+ tools     - ZigBee and IEEE 802.15.4 attack tools developed using this framework.

REQUIRED HARDWARE
================
The KillerBee framework is being expanded to support multiple devices.
Currently there is support for the River Loop ApiMote,
Atmel RZ RAVEN USB Stick, MoteIV Tmote Sky, TelosB mote, and Sewino Sniffer.

Support for Freaklab's Freakduino with added hardware
and the Dartmouth arduino sketch as well as for the Zena Packet Analyzer board
are in development.

ApiMote v3 and v4beta:
----------------
The devices typically come preloaded and do not need to be reflashed for basic
use.

MoteIV Tmote Sky or TelosB mode:
----------------
This device can be loaded with firmware via USB. Attach the device, and then
within killerbee/firmware, run:
```
$ ./flash_telosb.sh
```

Atmel RZ RAVEN USB Stick:
----------------
(http://www.atmel.com/dyn/products/tools_card.asp?tool_id=4396).  This hardware 
is convenient as the base firmware is open source with a freely-available IDE.
The KillerBee firmware for the RZ RAVEN included in the firmware/ directory is
a modified version of the stock firmware distributed by Atmel to include
attack functionality.

The RZ RAVEN USB Stick is available from common electronics resellers for
approximately $40/USD:

+ Mouser: http://bit.ly/vZ2pt
+ Digi-Key: http://bit.ly/3T8MaK

The stock firmware shipped with this hardware allows you to leverage the passive
functionality included in the KillerBee tools and framework (such as receiving
frames), but does not allow you to do packet injection, or to impersonate
devices on the network.

In order to get the full functionality included in KillerBee, the RZ RAVEN USB
Stick must be flashed with the custom firmware included in the firmware/ 
directory.  This process requires additional hardware and software:

  + Hardware: Atmel RZ Raven USB Stick (RZUSBSTICK)
  + Hardware: Atmel AVR Dragon On-Chip Programmer (ATAVRDRAGON)
  + Hardware: Atmel 100-mm to 50-mm JTAG Standoff Adapter (ATAVR-SOAKIT)
  + Hardware: 50mm male-to-male header (Digi-Key part S9015E-05)
  + Hardware: 10-pin (2x5) 100-mm female-to-female ribbon cable (Digi-Key part H3AAH-1018G-ND)
  + Software: AVRDUDE (http://winavr.sourceforge.net for Windows or http://www.nongnu.org/avrdude for Linux)
  + Software: KillerBee Firmware for the RZUSBSTICK
  + A Windows or Linux system for programming the RZ Raven USB Stick (one time operation)

For Windows users, install the AVR Dragon drivers provided with the libusb-win32 software
(http://sourceforge.net/projects/libusb-win32).  Download and extract the zip file, then launch the
libusb-win32 "inf-wizard.exe" executable.  Connect the AVR Dragon to a USB port and click Next in the wizard
to detect and identify the USB vendor ID and product ID 0x03EB and 0x2107 for the AVRDRAGON.  Complete the
wizard by clicking Next, then Finish to install the drivers.  When prompted by Windows, click "Install This
Driver Sofware Anyway".

Download the RZ Raven USB Stick firmware from 
https://raw.githubusercontent.com/riverloopsec/killerbee/master/firmware/kb-rzusbstick-002.hex.
Copy the firmware file to the directory where you extracted the AVRDUDE software.
Note: We are suggesting the -002 version now as some people report that -001 does not work on newer RZUSBSTICK versions. Change the filenames in the example commands below.

Connect the AVR Dragon programmer to the ribbon cable, and connect the 100-mm to 50-mm adapter with the
header.  Prepare your terminal to flash the RZ Raven USB stick by entering the following command at a
command prompt (but don't hit enter yet):

```
avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:kb-rzusbstick-001.hex
```

Insert the header into the RZ Raven USB Stick with pin 1 closest to the LEDs (farthest from the USB
connector).  You could solder it in place, but we don't bother.  Just hold it at an angle so all the pins
make contact and hit enter where you typed the AVRDUDE comand.  You should see output similar to the following:

```

C:\avrdude>avrdude -P usb -c dragon_jtag -p usb1287 -B 10 -U flash:w:kb-rzusbstick-001.hex

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

It should only take a few seconds to complete.  For a more detailed, picture-rich set of
instructions, grab a copy of Hacking Exposed Wireless 3rd Edition (chapter 13).  Alternatively,
if you are able to catch us at a conference, bring your RZ RAVEN USB Stick and we'll happily
flash it for you.


TOOLS
================
KillerBee includes several tools designed to attack ZigBee and IEEE 802.15.4
networks, built using the KillerBee framework.  Each tool has its own usage
instructions documented by running the tool with the "-h" argument, and
summarized below.


+ zbid         -  Identifies available interfaces that can be used by KillerBee
                and associated tools.
+ zbwireshark  -  Similar to zbdump but exposes a named pipe for real-time 
                capture and viewing in Wireshark.
+ zbdump       -  A tcpdump-like took to capture IEEE 802.15.4 frames to a libpcap
                or Daintree SNA packet capture file.  Does not display real-time
                stats like tcpdump when not writing to a file.
+ zbreplay     -  Implements a replay attack, reading from a specified Daintree
                DCF or libpcap packet capture file, retransmitting the frames.
                ACK frames are not retransmitted.
+ zbstumbler   -  Active ZigBee and IEEE 802.15.4 network discovery tool.
                Zbstumbler sends beacon request frames out while channel
                hopping, recording and displaying summarized information about
                discovered devices.  Can also log results to a CSV file.
+ zbpanidconflictflood  -  _Requires two killerbee interfaces_ one killerbee interface
                listens for packets and marks their PAN ID.  The other interface
                constantly sends out beacon packets with found PAN ID's.  The
                beacon packets with the same PAN ID cause the PAN coordinator to
                believe that there is a PAN ID conflict, and the coordinator begins
                the process of realigning the network on a new PAN ID.  The process
                repeats ad nauseum.  Typically, network devices can't keep up with
                the rapid change and after several seconds the network falls apart.

                _NO TARGETING BUILT IN_ This may *destroy* all zigbee networks
                within range on the channel you are performing the attack on. Use
                with caution.
+ zborphannotify  -  Spoofs an orphan notification packet from the target device to
                a PAN Coordinator to test Coordinator behavior.
+ zbrealign     -  Spoofs an 802.15.4 PAN Realignment frame from the coordinator to
                a target device.  May be able to reset the device's PAN ID or Channel
+ zbfakebeacon  -  Spoofs beacon frames, either spamming them or on response to seeing
                a beacon request come through.
+ zbopenear    -  Assists in data capture where devices are operating on multiple 
                channels or fast-frequency-hopping. It assigns multiple 
                interfaces sequentially across all channels.
+ zbassocflood -  Repeatedly associate to the target PANID in an effort to cause
                the device to crash from too many connected stations.
+ zbconvert    -  Convert a packet capture from Libpcap to Daintree SNA format,
                or vice-versa.
+ zbdsniff     -  Captures ZigBee traffic, looking for NWK frames and over-the-air
                key provisioning.  When a key is found, zbdsniff prints the
                key to stdout.  The sample packet capture
                sample/zigbee-network-key-ota.dcf can be used to demonstrate
                this functionality.
+ zbfind       -  A GTK GUI application for tracking the location of an IEEE
                802.15.4 transmitter by measuring RSSI.  Zbfind can be passive
                in discovery (only listen for packets) or it can be active by
                sending Beacon Request frames and recording the responses from
                ZigBee routers and coordinators.
                If you get a bunch of errors after starting this tool, make
                sure your DISPLAY variable is set properly.  If you know how
                to catch these errors to display a reasonable error message,
                please drop me a note.
+ zbgoodfind   -  Implements a key search function using an encrypted packet
                capture and memory dump from a legitimate ZigBee or IEEE
                802.15.4 device.  This tool accompanies Travis Goodspeed's
                GoodFET hardware attack tool, or other binary data that could
                contain encryption key information such as bus sniffing with
                legacy chips (such as the CC2420).  Zbgoodfind's search file
                must be in binary format (obj hexfile's are not supported). To
                convert from the hexfile format to a binary file, use the
                objcopy tool: objcopy -I ihex -O binary mem.hex mem.bin
+ zbwardrive   -	Discovers available interfaces and uses one to inject beacon 
                requests and listen for respones across channels. Once a network
                is found on a channel, it assigns another device to continuously
                capture traffic on that channel to a PCAP file. Scapy must be 
                installed to run this.
+ zbscapy      -  Provides an interactive Scapy shell for interacting via a
                KillerBee interface. Scapy must be installed to run this.

Additional tools, that are for special cases or are not stable, are stored in
    the Api-Do project repository: http://code.google.com/p/zigbee-security/


FRAMEWORK
==============
KillerBee is designed to simplify the process of sniffing packets from the air
interface or a supported packet capture file (libpcap or Daintree SNA), and for
injecting arbitrary packets.  Helper functions including IEEE 802.15.4, ZigBee
NWK and ZigBee APS packet decoders are available as well.

The KillerBee API is documented in epydoc format, with HTML documentation in 
the doc/ directory of this distribution.  If you have epydoc installed, you can
also generate a convenient PDF for printing, if desired, as shown:

```
$ cd killerbee
$ mkdir pdf
$ epydoc --pdf -o pdf killerbee/
```

The pdf/ directory will have a file called "api.pdf" which includes the
framework documentation.

To get started using the KillerBee framework, take a look at the included tools
(zbdump and zbreplay are good examples to get started) and the simple test
cases in the t/ directory.

Since KillerBee is a Python library, it integrates well with other Python
software as well.  For example, the Sulley library is a fuzzing framework
written in Python by Pedram Amini.  Using the Sulley mutation features and
KillerBee's packet injection features, it is staightforward to build a
mechanism for generating and transmitting malformed ZigBee data to a target.


QUESTIONS/COMMENTS/CONCERNS
==============
Please drop us a note: 

The original version was written by: jwright@willhackforsushi.com
The current version, fixes, etc are handled by: ryan@riverloopsecurity.com
Additional Tools/Fixes by: bryanhalf@gmail.com

THANKS
==============
A word of thanks to several folks who helped out with this project:

+ Travis Goodspeed
+ Mike Kershaw (dragorn)
+ Chris Wang (aikiba)
+ Nick DePetrillo
+ Ed Skoudis
+ Matt Carpenter
+ Sergey Bratus (research support at Dartmouth)
+ Jeff Spielberg

