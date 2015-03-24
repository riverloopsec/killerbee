# Introduction #

Some users may want to run KillerBee on Mac OS X. We offer some specific notes here.

# OS X + Fink #

This method was tested on Mac OS X 10.7.4, but should work for earlier versions as well. We recommend installing Fink, although applicable and related packages should be available under Macports as well, which has not been tested.

Once Fink has been installed (version 0.34.0 tested), install the following Fink packages, version tested listed for reference:
| **Package** | **Version (tested)** | **Description** |
|:------------|:---------------------|:----------------|
| pygtk2-gtk-py27 | 2.16.0-4 | Python bindings for the GTK widget set |
| pycairo-py27 | 1.8.10-1 | Python bindings for the cairo library |
| pyusb-py27 | 1.0.0a2-1 | Python access to libusb |
| pycrypto-py27 | 2.6-1 | Python Cryptographic Toolkit |
| pyserial-py27 | 2.6-1 | Python access to serial ports |

Additionally, python-dev headers are required, as well as libgcrypt, for the ScapyExtension Patch:
| py-py27 | 1.4.6-1 | Python development support library |
|:--------|:--------|:-----------------------------------|
| libgcrypt | 1.5.0-2  | GNU cryptographic library |

# Issues #

We note that currently only serial based devices, such as the GoodFET TelosB/Tmote and GoodFET Api-mote, are supported under OS X. This is due to differences in libusb and py-usb under OS X that are not cross-platform compatible. We are currently in the process of making USB support for devices such as the RZUSB stick cross-platform compatible.