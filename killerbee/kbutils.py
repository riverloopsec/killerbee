from typing import Optional, Dict, Union, List, Tuple, Any 

import sys

# Import USB support depending on version of pyUSB
import usb.core # type: ignore
import usb.util # type: ignore

import serial # type: ignore
import os
import struct
import glob
import time
import random
import inspect
from struct import pack

from .config import *       #to get DEV_ENABLE_* variables 

# Known devices by USB ID:
RZ_USB_VEND_ID: int       = 0x03EB
RZ_USB_PROD_ID: int       = 0x210A
ZN_USB_VEND_ID: int       = 0x04D8
ZN_USB_PROD_ID: int       = 0x000E
CC2530_USB_VEND_ID: int   = 0x11A0
CC2530_USB_PROD_ID: int   = 0xEB20
CC2531_USB_VEND_ID: int   = 0x0451
CC2531_USB_PROD_ID: int   = 0x16AE
BB_USB_VEND_ID: int       = 0x0451
BB_USB_PROD_ID: int       = 0x16A8
#FTDI_USB_VEND_ID      = 0x0403
#FTDI_USB_PROD_ID      = 0x6001 #this is also used by FDTI cables used to attach gps
FTDI_X_USB_VEND_ID: int   = 0x0403
FTDI_X_USB_PROD_ID: int   = 0x6015    #api-mote FTDI chip

usbVendorList: List[int] = [RZ_USB_VEND_ID, ZN_USB_VEND_ID, CC2530_USB_VEND_ID, CC2531_USB_VEND_ID, BB_USB_VEND_ID]
usbProductList: List[int] = [RZ_USB_PROD_ID, ZN_USB_PROD_ID, CC2530_USB_PROD_ID, CC2531_USB_PROD_ID, BB_USB_PROD_ID]

# Global variables
gps_devstring: Optional[str] = None


class KBCapabilities:
    """
    Class to store and report on the capabilities of a specific KillerBee device.
    """
    NONE: int          = 0x00 #: Capabilities Flag: No Capabilities
    SNIFF: int         = 0x01 #: Capabilities Flag: Can Sniff
    SETCHAN: int       = 0x02 #: Capabilities Flag: Can Set the Channel
    INJECT: int        = 0x03 #: Capabilities Flag: Can Inject Frames
    PHYJAM: int        = 0x04 #: Capabilities Flag: Can Jam PHY Layer
    SELFACK: int       = 0x05 #: Capabilities Flag: Can ACK Frames Automatically
    PHYJAM_REFLEX: int = 0x06 #: Capabilities Flag: Can Jam PHY Layer Reflexively
    SET_SYNC: int      = 0x07 #: Capabilities Flag: Can set the register controlling 802.15.4 sync byte
    FREQ_2400: int     = 0x08 #: Capabilities Flag: Can perform 2.4 GHz sniffing (ch 11-26)
    FREQ_900: int      = 0x09 #: Capabilities Flag: Can perform 900 MHz sniffing (ch 1-10)
    BOOT: int          = 0x0a #: Capabilities Flag: Has BootLoader
    FREQ_863: int      = 0x0b #: Capabilities Flag: Can perform 863-868 MHz sniffing (ch 0-26 )
    FREQ_868: int      = 0x0c #: Capabilities Flag: Can perform 868-876 MHz sniffing (ch 0-8)
    FREQ_870: int      = 0x0d #: Capabilities Flag: Can perform 870-876 MHz sniffing (ch 0-26)
    FREQ_915: int      = 0x0e #: Capabilities Flag: Can perform 915-917 MHz sniffing (ch 0-26)

    def __init__(self) -> None:
        self._capabilities: Dict[int, bool] = {
                self.NONE : False,
                self.SNIFF : False,
                self.SETCHAN : False,
                self.INJECT : False,
                self.PHYJAM : False,
                self.SELFACK: False,
                self.PHYJAM_REFLEX: False,
                self.SET_SYNC: False,
                self.FREQ_2400: False,
                self.FREQ_900: False ,
                self.FREQ_863: False,
                self.FREQ_868: False ,
                self.FREQ_870: False,
                self.FREQ_915: False,
                self.BOOT: False }

    def check(self, capab: int) -> bool:
        if capab in self._capabilities:
            return self._capabilities[capab]
        else:
            return False

    def getlist(self) -> Dict[int, bool]:
        return self._capabilities

    def setcapab(self, capab: int, value: bool) -> None:
        self._capabilities[capab] = value

    def require(self, capab: int) -> None:
        if self.check(capab) != True:
            raise Exception('Selected hardware does not support required capability (%d).' % capab)

    def frequency(self, channel: Optional[int]=None, page: int=0) -> int:
        '''
        Return actual frequency of channel/page in KHz
        '''
        if channel is None:
            return 0

        #TODO: FREQ_900
        if not self.is_valid_channel(channel, page):
            return 0
        #FREQ_2400
        if page == 0:
            base = 2405000
            step = 5000
            first = 11
        #FREQ_863
        if page == 28:
            base = 863250
            step = 200
            first = 0
        #FREQ_868
        if page == 29:
            base = 868650
            step = 200
            first = 0
        #FREQ_870
        if page == 30:
            base = 870250
            step = 200
            first = 0
        #FREQ_915
        if page == 31:
            base = 915350
            step = 200
            first = 0
        return (channel - first) * step + base

    def is_valid_channel(self, channel: Optional[int], page: int=0) -> bool:
        '''
        Based on sniffer capabilities, return if this is an OK channel number.
        @rtype: Boolean
        '''
        if channel is None:
            return False

        # if sub-ghz, check that page and channel and capability match
        if page:
            if page == 28 and (channel > 26 or not self.check(self.FREQ_863)):
                return False
            if page == 29 and (channel > 8 or not self.check(self.FREQ_868)):
                return False
            if page == 30 and (channel > 26 or not self.check(self.FREQ_870)):
                return False
            if page == 31 and (channel > 26 or not self.check(self.FREQ_915)):
                return False
            if page < 28 or page > 31:
                return False
            return True

        if (channel >= 11 and channel <= 26) and self.check(self.FREQ_2400):
            return True
        if (channel >= 1 and channel <= 10) and self.check(self.FREQ_900):
            return True
        return False

class findFromList(object):
    '''
    Custom matching function for pyUSB 1.x.
    Used by usb.core.find's custom_match parameter.
    '''
    def __init__(self, vendors_, products_) -> None:
        '''Takes a list of vendor IDs and product IDs.'''
        self._vendors: List[int]  = vendors_
        self._products: List[int] = products_

    def __call__(self, device: Any) -> bool:
        '''
        Returns True if the device being searched
        is in these lists.
        '''
        if (device.idVendor in self._vendors) and \
           (device.idProduct in self._products):
            return True

        return False

class findFromListAndBusDevId(findFromList):
    '''
    Custom matching function for pyUSB 1.x.
    Used by usb.core.find's custom_match parameter.
    '''
    def __init__(self, busNum_: Optional[int], devNum_: Optional[int], vendors_: List[int], products_: List[int]) -> None:
        '''Takes a list of vendor IDs and product IDs.'''
        findFromList.__init__(self, vendors_, products_)
        self._busNum: Optional[int] = busNum_
        self._devNum: Optional[int] = devNum_

    def __call__(self, device: Any) -> bool:
        '''
        Returns True if the device being searched
        is in these lists.
        '''
        if findFromList.__call__(self, device)                      and \
           (self._busNum == None or device.bus == self._busNum)     and \
           (self._devNum == None or device.address == self._devNum)     :
            return True

        return False

def devlist_usb_v1x(vendor: Optional[Any]=None, product: Optional[Any]=None) -> List[Any]:
    '''
    Private function. Do not call from tools/scripts/etc.
    '''
    devlist: List[Any] = []
    if vendor is None:  vendor = usbVendorList
    else:               vendor = [vendor]
    if product is None: product = usbProductList
    else:               product = [product]
    devs: Any = usb.core.find(find_all=True, custom_match=findFromList(vendor, product)) #backend=backend, 
    try:
        for dev in devs:
            # Note, can use "{0:03d}:{1:03d}" to get the old format,
            # but have decided to move to the new, shorter format.
            devlist.append(["{0}:{1}".format(dev.bus, dev.address),         \
                            usb.util.get_string(dev, dev.iProduct),     \
                            usb.util.get_string(dev, dev.iSerialNumber)])
    except usb.core.USBError as e:
        if e.errno == 13: #usb.core.USBError: [Errno 13] Access denied (insufficient permissions)
            raise Exception("Unable to open device. " +
                            "Ensure the device is free and plugged-in. You may need sudo.")
        else:
            raise e

    return devlist

def isIpAddr(ip: str) -> bool:
    '''Return True if the given string is a valid IPv4 or IPv6 address.'''
    import socket
    def is_valid_ipv4_address(address: str) -> bool:
        try:                    socket.inet_pton(socket.AF_INET, address)
        except AttributeError:  # no inet_pton here, sorry
            try:                    socket.inet_aton(address)
            except socket.error:    return False
            return (address.count('.') == 3)
        except socket.error:    return False
        return True
    def is_valid_ipv6_address(address: str) -> bool:
        try:                    socket.inet_pton(socket.AF_INET6, address)
        except socket.error:    return False
        return True
    return ( is_valid_ipv6_address(ip) or is_valid_ipv4_address(ip) )

def devlist(vendor: Optional[Any]=None, product: Optional[Any]=None, gps: Optional[str]=None, include: Optional[str]=None) -> List[Any]:
    '''
    Return device information for all present devices, 
    filtering if requested by vendor and/or product IDs on USB devices, and
    running device fingerprint functions on serial devices.
    @type gps: String
    @param gps: Optional serial device identifier for an attached GPS
        unit. If provided, or if global variable has previously been set, 
        KillerBee skips that device in device enumeration process.
    @type include: List of Strings
    @param include: Optional list of device handles to be appended to the 
        normally found devices. This is useful for providing IP addresses for
        remote scanners.
    @rtype: List
    @return: List of device information present.
                For USB devices, get [busdir:devfilename, productString, serialNumber]
                For serial devices, get [serialFileName, deviceDescription, ""]
    '''
    global usbVendorList, usbProductList, gps_devstring
    if gps is not None and gps_devstring is None:
        gps_devstring = gps

    devlist: List[Any] = devlist_usb_v1x(vendor, product)

    for serialdev in get_serial_ports(include=include):
        if serialdev == gps_devstring:
            continue
        elif (DEV_ENABLE_SL_NODETEST and issl_nodetest(serialdev)):
            devlist.append([serialdev, "Silabs NodeTest", ""])
        elif (DEV_ENABLE_SL_BEEHIVE and issl_beehive(serialdev)):
            devlist.append([serialdev, "BeeHive SG", ""])
        elif (DEV_ENABLE_ZIGDUINO and iszigduino(serialdev)):
            devlist.append([serialdev, "Zigduino", ""])
        elif (DEV_ENABLE_FREAKDUINO and isfreakduino(serialdev)):
            #TODO maybe move support for freakduino into goodfetccspi subtype==?
            devlist.append([serialdev, "Dartmouth Freakduino", ""])
        else:
            gfccspi,subtype = isgoodfetccspi(serialdev)
            if gfccspi and subtype == 0:
                devlist.append([serialdev, "GoodFET TelosB/Tmote", ""])
            elif gfccspi and subtype == 1:
                devlist.append([serialdev, "GoodFET Api-Mote v1", ""])
            elif gfccspi and subtype == 2:
                devlist.append([serialdev, "GoodFET Api-Mote v2", ""])
            elif gfccspi:
                print("kbutils.devlist has an unknown type of GoodFET CCSPI device ({0}).".format(serialdev))

    if include is not None:
        # Ugly nested load, so we don't load this class when unneeded!
        from . import dev_sewio #use isSewio, getFirmwareVersion
        for ipaddr in filter(isIpAddr, include):
            if dev_sewio.isSewio(ipaddr):
                devlist.append([ipaddr, "Sewio Open-Sniffer v{0}".format(dev_sewio.getFirmwareVersion(ipaddr)), dev_sewio.getMacAddr(ipaddr)])
            #NOTE: Enumerations of other IP connected sniffers go here.
            else:
                print("kbutils.devlist has an unknown type of IP sniffer device ({0}).".format(ipaddr))
    
    return devlist

def get_serial_devs(seriallist: List[str]) -> None:
    global DEV_ENABLE_FREAKDUINO, DEV_ENABLE_ZIGDUINO
    #TODO Continue moving code from line 163:181 here, yielding results

def isSerialDeviceString(s: str) -> bool:
    return ( ( s.count('/') + s.count('tty') ) > 0 )

def get_serial_ports(include: Optional[Any]=None) -> Any:
    '''
    Private function. Do not call from tools/scripts/etc.
    This should return a list of device paths for serial devices that we are
    interested in, aka USB serial devices using FTDI chips such as the TelosB,
    ApiMote, etc. This should handle returning a list of devices regardless of
    the *nix it is running on. Support for more *nix and winnt needed.
    
    @type include: List of Strings, or None
    @param include: A list of device strings, of which any which appear to be
        serial device handles will be added to the set of serial ports returned
        by the normal search. This may be useful if we're not including some
        oddly named serial port which you have a KillerBee device on. Optional.
    '''
    seriallist = glob.glob("/dev/ttyUSB*") + glob.glob("/dev/tty.usbserial*") + glob.glob("/dev/ttyACM*") #TODO make cross platform globing/winnt
    if include is not None:
        seriallist = list( set(seriallist).union(set(filter(isSerialDeviceString, include))) )
    return seriallist

def isgoodfetccspi(serialdev: str) -> Tuple[bool, Optional[int]]:
    '''
    Determine if a given serial device is running the GoodFET firmware with the CCSPI application.
    This should either be a TelosB/Tmote Sky GOODFET or an Api-Mote design.
    @type serialdev:  String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype:   Tuple
    @returns: Tuple with the fist element==True if it is some goodfetccspi device. The second element
                is the subtype, and is 0 for telosb devices and 1 for apimote devices.
    '''
    #TODO reduce code, perhaps into loop iterating over board configs
    from .GoodFETCCSPI import GoodFETCCSPI
    os.environ["platform"] = ""
    # First try tmote detection
    if DEV_ENABLE_TELOSB:
        os.environ["board"] = "telosb" #set enviroment variable for GoodFET code to use
        gf = GoodFETCCSPI()
        try:
            gf.serInit(port=serialdev, attemptlimit=2)
        except serial.serialutil.SerialException as e:
            raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)
        if gf.connected == 1:
            # now check if ccspi app is installed
            out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
            gf.serClose()
            if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
                return True, 0
    # Try apimote v2 detection
    if DEV_ENABLE_APIMOTE2:
        os.environ["board"] = "apimote2" #set enviroment variable for GoodFET code to use
        gf = GoodFETCCSPI()
        try:
            gf.serInit(port=serialdev, attemptlimit=30)
            #gf.setup()
        except serial.serialutil.SerialException as e:
            raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)
        if gf.connected == 1:
            # now check if ccspi app is installed
            out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
            gf.serClose()
            if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
                return True, 2
    # Then try apimote v1 detection
    if DEV_ENABLE_APIMOTE1:
        os.environ["board"] = "apimote1" #set enviroment variable for GoodFET code to use
        gf = GoodFETCCSPI()
        try:
            #TODO note that in ApiMote v1, this connect appears to be tricky sometimes
            #     thus attempt limit is raised to 4 for now
            #     manually verify the hardware is working by using direct GoodFET client commands, such as:
            #       export board=apimote1; ./goodfet.ccspi info; ./goodfet.ccspi spectrum
            gf.serInit(port=serialdev, attemptlimit=4)
            #gf.setup()
        except serial.serialutil.SerialException as e:
            raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)
        if gf.connected == 1:
            # now check if ccspi app is installed
            out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
            gf.serClose()
            if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
                return True, 1
    # Nothing found
    return False, None

def iszigduino(serialdev: str) -> bool:
    '''
    Determine if a given serial device is running the GoodFET firmware with the atmel_radio application.
    This should be a Zigduino (only tested on hardware r1 currently).
    @type serialdev:  String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype:   Boolean
    @returns: Boolean with the fist element==True if it is a goodfet atmel128 device.
    '''
    # TODO why does this only work every-other time zbid is invoked?
    from .GoodFETatmel128 import GoodFETatmel128rfa1
    os.environ["platform"] = "zigduino"
    gf = GoodFETatmel128rfa1()
    try:
        gf.serInit(port=serialdev, attemptlimit=2)
    except serial.serialutil.SerialException as e:
        raise KBInterfaceError("Serial issue in kbutils.iszigduino: %s." % e)
    if gf.connected == 1:
        out = gf.writecmd(gf.ATMELRADIOAPP, 0x10, 0, None)
        gf.serClose()
        if (gf.app == gf.ATMELRADIOAPP) and (gf.verb == 0x10): #check if ATMELRADIOAPP exists           
            return True
    return False
    
def issl_nodetest(serialdev: str) -> bool:
    '''
    Determine if a given serial device is a Silabs dev board NodeTest loaded (https://www.silabs.com/documents/public/application-notes/AN1019-NodeTest.pdf)
    @type serialdev: String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype: Boolean
    '''
    s: serial.Serial = serial.Serial(port=serialdev, baudrate=115200, timeout=.1, bytesize=8, parity='N', stopbits=1, xonxoff=0)

    s.write(b'\re\r')
    for x in range(5):
        s.readline()
    s.write(b'version\r')
    version = None
    for i in range(5):
        d = s.readline()
        if b'Node Test Application' in d:
            version = d
            break
    s.close()
    return (version is not None)

def issl_beehive(serialdev: str) -> bool:
    '''
    Determine if a given serial device is a BeeHive SG - contact Adam Laurie <adam@algroup.co.uk> for more info
    @type serialdev: String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype: Boolean
    '''
    s: serial.Serial = serial.Serial(port=serialdev, baudrate=115200, timeout=.5, bytesize=8, parity='N', stopbits=1, xonxoff=0)

    s.write(b'\rrx 0\r')
    while s.in_waiting:
        d = s.readline()
    s.write(b'\r')
    version = None
    for i in range(5):
        d = s.readline()
        if b'BeeHive SG' in d:
            version = d
            break
    s.close()
    return (version is not None)

def isfreakduino(serialdev: str) -> bool:
    '''
    Determine if a given serial device is a Freakduino attached with the right sketch loaded.
    @type serialdev: String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype: Boolean
    '''
    s: serial.Serial = serial.Serial(port=serialdev, baudrate=57600, timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
    time.sleep(1.5)
    s.write(b'SC!V\r')
    time.sleep(1.5)
    #readline should take an eol argument, per:
    # http://pyserial.sourceforge.net/pyserial_api.html#serial.FileLike.readline
    # However, many got an "TypeError: readline() takes no keyword arguments" due to a pySerial error
    # So we have replaced it with a bruteforce method. Old: s.readline(eol='&')
    for i in range(100):
        if (s.read() == b'&'): break
    if s.read(3) == b'C!V': version = s.read()
    else: version = None
    s.close()
    return (version is not None)

def search_usb(device: Any) -> Any:
    """
    Takes either None, specifying that any USB device in the
    global vendor and product lists are acceptable, or takes
    a string that identifies a device in the format
    <BusNumber>:<DeviceNumber>, and returns the pyUSB objects
    for bus and device that correspond to the identifier string.
    """
    if device == None:
        busNum: Optional[int] = None
        devNum: Optional[int] = None
    else:
        if ':' not in device:
            raise KBInterfaceError("USB device format expects <BusNumber>:<DeviceNumber>, but got {0} instead.".format(device))
        busNum, devNum = list(map(int, device.split(':', 1)))
    return usb.core.find(custom_match=findFromListAndBusDevId(busNum, devNum, usbVendorList, usbProductList)) #backend=backend, 

def hexdump(src, length=16):
    '''
    Creates a tcpdump-style hex dump string output.
    @type src: String
    @param src: Input string to convert to hexdump output.
    @type length: Int
    @param length: Optional length of data for a single row of output, def=16
    @rtype: String
    '''
    FILTER = b''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])
    result = []
    for i in range(0, len(src), length):
       chars = src[i:i+length]
       hex = ' '.join(["%02x" % ord(x) for x in chars])
       printable = ''.join(["%s" % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
       result.append("%04x:  %-*s  %s\n" % (i, length*3, hex, printable))
    return ''.join(result)


def randbytes(size: int) -> str:
    '''
    Returns a random string of size bytes.  Not cryptographically safe.
    @type size: Int
    @param size: Length of random data to return.
    @rtype: String
    '''
    return ''.join(chr(random.randrange(0,256)) for i in range(size))


def randmac(length: int=8) -> str:
    '''
    Returns a random MAC address using a list valid OUI's from ZigBee device 
    manufacturers.  Data is returned in air-format byte order (LSB first).
    @type length: String
    @param length: Optional length of MAC address, def=8.  
        Minimum address return length is 3 bytes for the valid OUI.
    @rtype: String
    @returns: A randomized MAC address in a little-endian byte string.
    '''
    # Valid OUI prefixes for MAC addresses
    prefixes: List[str] = [ 
                "\x00\x0d\x6f",     # Ember
                "\x00\x12\x4b",     # TI
                "\x00\x04\xa3",     # Microchip
                "\x00\x04\x25",     # Atmel
                "\x00\x11\x7d",     # ZMD
                "\x00\x13\xa2",     # MaxStream
                "\x00\x30\x66",     # Cirronet
                "\x00\x0b\x57",     # Silicon Laboratories
                "\x00\x04\x9f",     # Freescale Semiconductor
                "\x00\x21\xed",     # Telegesis
                "\x00\xa0\x50"      # Cypress
                ]

    prefix: str = random.choice(prefixes)
    suffix: str = randbytes(length-3)
    # Reverse the address for use in a packet
    return ''.join([prefix, suffix])[::-1]


def makeFCS(data: bytes) -> bytes:
    '''
    Do a CRC-CCITT Kermit 16bit on the data given
    Implemented using pseudocode from: June 1986, Kermit Protocol Manual
    See also: http://regregex.bbcmicro.net/crc-catalogue.htm#crc.cat.kermit

    @return: a CRC that is the FCS for the frame, as two hex bytes in
        little-endian order.
    '''
    crc: int = 0
    for c in bytearray(data):
        #if (A PARITY BIT EXISTS): c = c & 127	#Mask off any parity bit
        q = (crc ^ c) & 15				#Do low-order 4 bits
        crc = (crc // 16) ^ (q * 4225)
        q = (crc ^ (c // 16)) & 15		#And high 4 bits
        crc = (crc // 16) ^ (q * 4225)
    return pack('<H', crc) #return as bytes in little endian order

class KBException(Exception):
    '''Base class for all KillerBee specific exceptions.'''
    pass


class KBInterfaceError(KBException):
    '''
    Custom exception for KillerBee having issues communicating
    with an interface, such as opening a port, syncing with the firmware, etc.
    '''
    pass

def bytearray_to_bytes(b: List[bytes]) -> bytes:
    return b"".join(struct.pack('B', value) for value in b)
