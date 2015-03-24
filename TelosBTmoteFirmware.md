# Introduction #
The firmware used is from the GoodFET project, compiled for the telosb platform and with ccspi radio functionality included.

# Loading Precompiled (KillerBee repository) Firmware Image #
This method is **highly suggested** to avoid dependency requirements. _Note, however, that this firmware may become out-of-date and require updates._

_Within killerbee/firmware, run:_
  * ./goodfet.bsl --telosb -e -p gf-telosb-001.hex

# Compiling Firmware Image #
**Note:**
  * This step is **not necessary** if you use the pre-compiled GoodFET firmware included in the killerbee/firmware directory.
  * Compiling the firmware yourself requires the GoodFET SVN project and the msp430-gcc compiler and related dependencies to be installed on your system.
  * Tested at GoodFET rev. 1131, it no longer builds with msp430-gcc-4.4.5 as the build scripts have been changed to support the new msp430-gcc which is not backwards compatible. Thus:
  * This requires msp430-gcc version 4.5.3. On Ubuntu 11.10, this can be installed via apt-get. Verify msp430-gcc version if you have issues.

_Within goodfet/trunk/firmware, run:_

platform=telosb make clean install

_If make fails on goodfet.bsl command not found, simply run:_
  * ../client/goodfet.bsl --speed=38400 -e -p goodfet.hex

# Verify Firmware Success #
_Within goodfet/trunk/client, run:_

goodfet.ccspi info

goodfet.spiflash info