# Introduction #

KillerBee provides an interface for experimenting with the security of ZigBee and IEEE 802.15.4 networks.

# Install #
You can get the most recent source code from the SVN repository. Releases are in /tags/ and current development (may have issues) is in /trunk/. Also, tar archives of releases are available on the downloads page. Follow the README in the main directory for installation guidance.

# Supported Hardware #
We aim to keep adding support for more hardware. We have recently added a device "driver" model to make this easier. If you would like to help or have a suggestion of hardware to be supported, please contact us.
  * _Atmel RZUSBSTICK_ [Atmel Store $39](http://store.atmel.com/PartDetail.aspx?q=p:10500060)
    * Note that stock firmware provides sniffing only. Contact us if you want assistance flashing the KillerBee firmware.
    * The Api-Do project has cases available that make the board more suitable for pen-testing/field-work.
  * _MoteIV Tmote Sky_ or _TelosB_ mote
    * Not in active production. We are working on a solution. Currently available used and typically abundant at universities.
    * We've had luck with European-sold clones, flashed with our same firmware.
  * _Api-Mote_ (in beta)
    * First revision had good success, working on future revisions of the hardware.
    * Runs a ported GoodFET firmware base.
  * _Zena Packet Analyzer_ (in development)
  * _Freaklabs Freakduino_ with Dartmouth modifications (in beta)