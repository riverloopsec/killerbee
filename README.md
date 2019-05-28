KillerBee
================

This is KillerBee - Framework and Tools for Attacking ZigBee and IEEE 802.15.4 networks.

MAINTAINERS/LICENSE
================

Distributed under a BSD license, see LICENSE.txt for details.
All Rights Reserved.

The main toolkit was/is authored by:
+ 2009, Joshua Wright <jwright@willhackforsushi.com>
+ 2010-2019, Ryan Speers <ryan@riverloopsecurity.com>
+ 2010-2011, Ricky Melgares <ricky@riverloopsecurity.com>

We appreciate the many contributers to the framework, including the following who have contributed capabilities:
+ Anonymous Contributors
+ Spencer McIntyre (scapy extension)
+ Bryan Halfpap <Bryanhalf@gmail.com> (additional tools)
+ Travis Goodspeed
+ Mike Kershaw (dragorn)
+ Chris Wang (aikiba)
+ Nick DePetrillo
+ Ed Skoudis
+ Matt Carpenter
+ Sergey Bratus (research support at Dartmouth)
+ Jeff Spielberg
+ Scytmo (bug fixes and CC2530/1 EMK board support)
+ Adam Laurie/rfidiot (APS crypto implementation, firmware, DFU & BOOTLOADER, SubGHZ, SiLabs NodeTest)
+ Steve Martin

REQUIREMENTS
================

KillerBee is developed and tested on Linux systems.
MacOS usage is possible but not supported.

We have striven to use a minimum number of software dependencies, however, it
is necessary to install the following Python modules before installation.
The install will detect and prompt you for what is needed.

On Ubuntu systems, you can install the needed dependencies with the following
commands:

```
# apt-get install python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev
# git clone https://github.com/secdev/scapy
# cd scapy
# python setup.py install
```

The python-dev and libgcrypt are required for the Scapy Extension Patch.

Also note that this is a fairly advanced and un-friendly attack platform.  This
is not Cain & Abel.  It is intended for developers and advanced analysts who are
attacking ZigBee and IEEE 802.15.4 networks.  I recommend you gain some
understanding of the ZigBee protocol (the book [ZigBee Wireless Networks and
Transceivers by Shahin Farahani](http://bit.ly/2I5ppI) is reasonable, though
still not great) and familiarity with the Python language before digging into
this framework.


INSTALLATION
================
KillerBee uses the standard Python 'setup.py' installation file.
Install KillerBee with the following command:

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
Currently there is support for the River Loop ApiMote, Atmel RZ RAVEN USB Stick,
MoteIV Tmote Sky, TelosB mote, Sewino Sniffer, and various hardware running Silicon Labs Node Test firmware.

Support for Freaklab's Freakduino with added hardware & the Dartmouth arduino sketch
and Zigduino boards are available but are not listed below as they are not maintained.
You must enable these to be searched for in `killerbee/config.py` and then reinstall KillerBee.

ApiMote v4beta (and v3):
----------------
The devices typically come preloaded and do not need to be reflashed for basic use.

The hardware is open-source at https://github.com/riverloopsec/apimote.
It is available assembled by contacting team at riverloopsecurity dot com.

_This is currently supported for beta, and supports sniffing, injection, and jamming._

Texas Instruments CC2530/1 EMK:
----------------

This USB dongle is produced by Texas Instruments and is sold as an evaluation
kit for their CC2530 or CC2531 integrated circuit.

It can be purchased from electronics distributors, or directly from them
[here](http://www.ti.com/tool/cc2531emk).

_This is currently supported for beta, and supports sniffing only._

MoteIV Tmote Sky or TelosB mode:
----------------
This device can be loaded with firmware via USB. Attach the device, and then
within killerbee/firmware, run:
```
$ ./flash_telosb.sh
```

These boards can be obtained via multiple distributors, however
[this vendor](https://www.advanticsys.com/shop/mtmcm5000msp-p-14.html) has
stated that their "clone" of the original hardware is compatible.
We have not tested nor do we endorse any specific "clone".

Atmel RZ RAVEN USB Stick:
----------------
See http://www.atmel.com/tools/RZUSBSTICK.aspx.
This hardware 
is convenient as the base firmware is open source with a freely-available IDE.
The KillerBee firmware for the RZ RAVEN included in the `firmware/` directory is
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
Stick must be flashed with the custom firmware included in the `firmware/ `
directory. _See [firmware/README.md](firmware/README.md) for details._

Silicon Labs Node Test 2.4GHz & SubGHz:
----------------
See [SiLabs AN1019](https://www.silabs.com/documents/public/application-notes/AN1019-NodeTest.pdf).

This is a firmware image for a huge variety of hardware platforms, supporting EM250, EM375, EFR32, etc., and provides for (with appropriate radio part) the following frequencies:
+ 2.4 GHz (ch 11-26)
+ 863-917 MHz (pages 28-31, ch 0-26)

You must enable these to be searched for in `killerbee/config.py` and then reinstall KillerBee.
See [firmware/README.md](firmware/README.md) for installation details.

_NOTE: This is a SNIFFER only implementation which will only read packets with a good FCS, and although the firmware has injection capability, it adds a 2 byte counter to the end of every packet, rendering it useless for all practical purposes. If you are interested in an INJECTION and BAD FCS capable device, contact the author, Adam Laurie - adam at algroup.co.uk_

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
                `sample/zigbee-network-key-ota.dcf` can be used to demonstrate
                this functionality.
+ zbfind       -  A GTK GUI application for tracking the location of an IEEE
                802.15.4 transmitter by measuring RSSI. zbfind can be passive
                in discovery (only listen for packets) or it can be active by
                sending Beacon Request frames and recording the responses from
                ZigBee routers and coordinators.
                If you get a bunch of errors after starting this tool, make
                sure your `DISPLAY` variable is set properly.
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
+ kbbootloader -  Switches device into DFU/BOOTLOADER mode (if device is capable)

Additional tools, that are for special cases or are not stable, are stored in
    the Api-Do project repository: http://code.google.com/p/zigbee-security/
    and at https://github.com/riverloopsec/beekeeperwids.


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
(zbdump and zbreplay are good examples to get started).

Since KillerBee is a Python library, it integrates well with other Python
software as well.  For example, the Sulley library is a fuzzing framework
written in Python by Pedram Amini.  Using the Sulley mutation features and
KillerBee's packet injection features, it is staightforward to build a
mechanism for generating and transmitting malformed ZigBee data to a target.


QUESTIONS/COMMENTS/CONCERNS
==============
Please use the ticketing system at https://github.com/riverloopsec/killerbee/issues.

The original version was written by: jwright@willhackforsushi.com.
The current version, fixes, etc are handled by: ryan@riverloopsecurity.com.
(See the list above for all contributors/credits.)

For contributors/developers, see `DEVELOPMENT.md` for details and guidance.
