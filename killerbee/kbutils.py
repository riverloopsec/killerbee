# Import USB support depending on version of pyUSB
try:
    import usb.core
    import usb.util
    #import usb.backend.libusb01
    #backend = usb.backend.libusb01.get_backend()
    USBVER=1
except ImportError:
    import usb
    #print("Warning: You are using pyUSB 0.x, future deprecation planned.")
    USBVER=0

import serial
import os, glob
import time
import random
import inspect
from struct import pack

from config import *       #to get DEV_ENABLE_* variables

# Known devices by USB ID:
RZ_USB_VEND_ID      = 0x03EB
RZ_USB_PROD_ID      = 0x210A
ZN_USB_VEND_ID      = 0x04D8
ZN_USB_PROD_ID      = 0x000E
#FTDI_USB_VEND_ID      = 0x0403
#FTDI_USB_PROD_ID      = 0x6001 #this is also used by FDTI cables used to attach gps
FTDI_X_USB_VEND_ID  = 0x0403
FTDI_X_USB_PROD_ID  = 0x6015    #api-mote FTDI chip

usbVendorList  = [RZ_USB_VEND_ID, ZN_USB_VEND_ID]
usbProductList = [RZ_USB_PROD_ID, ZN_USB_PROD_ID]

# Global variables
gps_devstring = None

class KBCapabilities:
    '''
    Class to store and report on the capabilities of a specific KillerBee device.
    '''
    NONE               = 0x00 #: Capabilities Flag: No Capabilities
    SNIFF              = 0x01 #: Capabilities Flag: Can Sniff
    SETCHAN            = 0x02 #: Capabilities Flag: Can Set the Channel
    INJECT             = 0x03 #: Capabilities Flag: Can Inject Frames
    PHYJAM             = 0x04 #: Capabilities Flag: Can Jam PHY Layer
    SELFACK            = 0x05 #: Capabilities Flag: Can ACK Frames Automatically
    PHYJAM_REFLEX      = 0x06 #: Capabilities Flag: Can Jam PHY Layer Reflexively
    SET_SYNC           = 0x07 #: Capabilities Flag: Can set the register controlling 802.15.4 sync byte
    FREQ_2400          = 0x08 #: Capabilities Flag: Can preform 2.4 GHz sniffing (ch 11-26)
    FREQ_900           = 0x09 #: Capabilities Flag: Can preform 900 MHz sniffing (ch 1-10)
    def __init__(self):
        self._capabilities = {
                self.NONE : False,
                self.SNIFF : False,
                self.SETCHAN : False,
                self.INJECT : False,
                self.PHYJAM : False,
                self.SELFACK: False,
                self.PHYJAM_REFLEX: False,
                self.SET_SYNC: False,
                self.FREQ_2400: False,
                self.FREQ_900: False }
    def check(self, capab):
        if capab in self._capabilities:
            return self._capabilities[capab]
        else:
            return False
    def getlist(self):
        return self._capabilities
    def setcapab(self, capab, value):
        self._capabilities[capab] = value
    def require(self, capab):
        if self.check(capab) != True:
            raise Exception('Selected hardware does not support required capability (%d).' % capab)
    def is_valid_channel(self, channel):
        '''
        Based on sniffer capabilities, return if this is an OK channel number.
        @rtype: Boolean
        '''
        if (channel >= 11 or channel <= 26) and self.check(self.FREQ_2400):
            return True
        elif (channel >= 1 or channel <= 10) and self.check(self.FREQ_900):
            return True
        return False

class findFromList(object):
    '''
    Custom matching function for pyUSB 1.x.
    Used by usb.core.find's custom_match parameter.
    '''
    def __init__(self, vendors_, products_):
        '''Takes a list of vendor IDs and product IDs.'''
        self._vendors  = vendors_
        self._products = products_
    def __call__(self, device):
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
    def __init__(self, busNum_, devNum_, vendors_, products_):
        '''Takes a list of vendor IDs and product IDs.'''
        findFromList.__init__(self, vendors_, products_)
        self._busNum = busNum_
        self._devNum = devNum_
    def __call__(self, device):
        '''
        Returns True if the device being searched
        is in these lists.
        '''
        if findFromList.__call__(self, device)                      and \
           (self._busNum == None or device.bus == self._busNum)     and \
           (self._devNum == None or device.address == self._devNum)     :
            return True
        return False

def devlist_usb_v1x(vendor=None, product=None):
    '''
    Private function. Do not call from tools/scripts/etc.
    '''
    devlist = []
    if vendor == None:  vendor = usbVendorList
    else:               vendor = [vendor]
    if product == None: product = usbProductList
    else:               product = [product]
    devs = usb.core.find(find_all=True, custom_match=findFromList(vendor, product)) #backend=backend, 
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

def devlist_usb_v0x(vendor=None, product=None):
    '''
    Private function. Do not call from tools/scripts/etc.
    '''
    devlist = []
    busses = usb.busses()
    for bus in busses:
        devices = bus.devices
        for dev in devices:
            if ((vendor==None and dev.idVendor in usbVendorList) or dev.idVendor==vendor) \
               and ((product==None and dev.idProduct in usbProductList) or dev.idProduct==product):
                devlist.append([''.join([bus.dirname + ":" + dev.filename]), \
                                  dev.open().getString(dev.iProduct, 50),    \
                                  dev.open().getString(dev.iSerialNumber, 12)])
    return devlist

def isIpAddr(ip):
    '''Return True if the given string is a valid IPv4 or IPv6 address.'''
    import socket
    def is_valid_ipv4_address(address):
        try:                    socket.inet_pton(socket.AF_INET, address)
        except AttributeError:  # no inet_pton here, sorry
            try:                    socket.inet_aton(address)
            except socket.error:    return False
            return (address.count('.') == 3)
        except socket.error:    return False
        return True
    def is_valid_ipv6_address(address):
        try:                    socket.inet_pton(socket.AF_INET6, address)
        except socket.error:    return False
        return True
    return ( is_valid_ipv6_address(ip) or is_valid_ipv4_address(ip) )

def devlist(vendor=None, product=None, gps=None, include=None):
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
    devlist = []

    if USBVER == 0:
        devlist = devlist_usb_v0x(vendor, product)
    elif USBVER == 1:
        devlist = devlist_usb_v1x(vendor, product)

    for serialdev in get_serial_ports(include=include):
        if serialdev == gps_devstring:
            print("kbutils.devlist is skipping ignored/GPS device string {0}".format(serialdev)) #TODO remove debugging print
            continue
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
        import dev_sewio #use isSewio, getFirmwareVersion
        for ipaddr in filter(isIpAddr, include):
            if dev_sewio.isSewio(ipaddr):
                devlist.append([ipaddr, "Sewio Open-Sniffer v{0}".format(dev_sewio.getFirmwareVersion(ipaddr)), dev_sewio.getMacAddr(ipaddr)])
            #NOTE: Enumerations of other IP connected sniffers go here.
            else:
                print("kbutils.devlist has an unknown type of IP sniffer device ({0}).".format(ipaddr))
    
    return devlist

def get_serial_devs(seriallist):
    global DEV_ENABLE_FREAKDUINO, DEV_ENABLE_ZIGDUINO
    #TODO Continue moving code from line 163:181 here, yielding results

def isSerialDeviceString(s):
    return ( ( s.count('/') + s.count('tty') ) > 0 )

def get_serial_ports(include=None):
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
    seriallist = glob.glob("/dev/ttyUSB*") + glob.glob("/dev/tty.usbserial*")  #TODO make cross platform globing/winnt
    if include is not None:
        seriallist = list( set(seriallist).union(set(filter(isSerialDeviceString, include))) )
    return seriallist

def isgoodfetccspi(serialdev):
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
    from GoodFETCCSPI import GoodFETCCSPI
    os.environ["platform"] = ""
    # First try tmote detection
    os.environ["board"] = "telosb" #set enviroment variable for GoodFET code to use
    gf = GoodFETCCSPI()
    try:
        gf.serInit(port=serialdev, attemptlimit=2)
    except serial.serialutil.SerialException as e:
        raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)
    if gf.connected == 1:
        #print "TelosB/Tmote attempts: found %s on %s" % (gf.identstr(), serialdev)
        # now check if ccspi app is installed
        out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
        gf.serClose()        
        if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
            return True, 0
    # Try apimote v2 detection
    os.environ["board"] = "apimote2" #set enviroment variable for GoodFET code to use
    gf = GoodFETCCSPI()
    try:
        gf.serInit(port=serialdev, attemptlimit=2)
        #gf.setup()
    except serial.serialutil.SerialException as e:
        raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)    
    if gf.connected == 1:
        #print "ApiMotev2+ attempts: found %s on %s" % (gf.identstr(), serialdev)
        # now check if ccspi app is installed
        out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
        gf.serClose()        
        if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
            return True, 2
    # Then try apimote v1 detection
    os.environ["board"] = "apimote1" #set enviroment variable for GoodFET code to use
    gf = GoodFETCCSPI()
    try:
        #TODO note that in ApiMote v1, this connect appears to be tricky sometimes
        #     thus attempt limit is raised to 4 for now
        #     manually verify the hardware is working by using direct GoodFET client commands, such as:
        #       export board=apimote1; ./goodfet.ccspi info; ./goodfet.ccspi spectrum
        gf.serInit(port=serialdev, attemptlimit=4)
        #gf.setup()
        #print "Found %s on %s" % (gf.identstr(), serialdev)
    except serial.serialutil.SerialException as e:
        raise KBInterfaceError("Serial issue in kbutils.isgoodfetccspi: %s." % e)    
    if gf.connected == 1:
        #print "ApiMotev1 attempts: found %s on %s" % (gf.identstr(), serialdev)
        # now check if ccspi app is installed
        out = gf.writecmd(gf.CCSPIAPP, 0, 0, None)
        gf.serClose()        
        if (gf.app == gf.CCSPIAPP) and (gf.verb == 0):
            return True, 1
    # Nothing found
    return False, None

def iszigduino(serialdev):
    '''
    Determine if a given serial device is running the GoodFET firmware with the atmel_radio application.
    This should be a Zigduino (only tested on hardware r1 currently).
    @type serialdev:  String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype:   Boolean
    @returns: Boolean with the fist element==True if it is a goodfet atmel128 device.
    '''
    # TODO why does this only work every-other time zbid is invoked?
    from GoodFETatmel128 import GoodFETatmel128rfa1
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
    
def isfreakduino(serialdev):
    '''
    Determine if a given serial device is a Freakduino attached with the right sketch loaded.
    @type serialdev: String
    @param serialdev: Path to a serial device, ex /dev/ttyUSB0.
    @rtype: Boolean
    '''
    s = serial.Serial(port=serialdev, baudrate=57600, timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
    time.sleep(1.5)
    s.write('SC!V\r')
    time.sleep(1.5)
    #readline should take an eol argument, per:
    # http://pyserial.sourceforge.net/pyserial_api.html#serial.FileLike.readline
    # However, many got an "TypeError: readline() takes no keyword arguments" due to a pySerial error
    # So we have replaced it with a bruteforce method. Old: s.readline(eol='&')
    for i in range(100):
        if (s.read() == '&'): break
    if s.read(3) == 'C!V': version = s.read()
    else: version = None
    s.close()
    return (version is not None)

def search_usb(device):
    '''
    Takes either None, specifying that any USB device in the
    global vendor and product lists are acceptable, or takes
    a string that identifies a device in the format
    <BusNumber>:<DeviceNumber>, and returns the pyUSB objects
    for bus and device that correspond to the identifier string.
    '''
    if device == None:
        busNum = None
        devNum = None
    else:
        if ':' not in device:
            raise KBInterfaceError("USB device format expects <BusNumber>:<DeviceNumber>, but got {0} instead.".format(device))
        busNum, devNum = map(int, device.split(':', 1))
    if USBVER == 0:
        busses = usb.busses()
        for bus in busses:
            dev = search_usb_bus_v0x(bus, busNum, devNum)
            if dev != None:
                return (bus, dev)
        return None #Note, can't expect a tuple returned
    elif USBVER == 1:
        return usb.core.find(custom_match=findFromListAndBusDevId(busNum, devNum, usbVendorList, usbProductList)) #backend=backend, 
    else:
        raise Exception("USB version expected to be 0.x or 1.x.")

def search_usb_bus_v0x(bus, busNum, devNum):
    '''Helper function for USB enumeration in pyUSB 0.x enviroments.'''
    devices = bus.devices
    for dev in devices:
        if (dev.idVendor in usbVendorList) and (dev.idProduct in usbProductList):
            #Populate the capability information for this device later, when driver is initialized
            if devNum == None:
                return dev
            elif busNum == int(bus.dirname) and devNum == int(dev.filename):
                #print "Choose device", bus.dirname, dev.filename, "to initialize KillerBee instance on."
                return dev
    return None

def hexdump(src, length=16):
    '''
    Creates a tcpdump-style hex dump string output.
    @type src: String
    @param src: Input string to convert to hexdump output.
    @type length: Int
    @param length: Optional length of data for a single row of output, def=16
    @rtype: String
    '''
    FILTER = ''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])
    result = []
    for i in xrange(0, len(src), length):
       chars = src[i:i+length]
       hex = ' '.join(["%02x" % ord(x) for x in chars])
       printable = ''.join(["%s" % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
       result.append("%04x:  %-*s  %s\n" % (i, length*3, hex, printable))
    return ''.join(result)

def randbytes(size):
    '''
    Returns a random string of size bytes.  Not cryptographically safe.
    @type size: Int
    @param size: Length of random data to return.
    @rtype: String
    '''
    return ''.join(chr(random.randrange(0,256)) for i in xrange(size))

def randmac(length=8):
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
    prefixes = [ "\x00\x0d\x6f",    # Ember
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

    prefix = random.choice(prefixes)
    suffix = randbytes(length-3)
    # Reverse the address for use in a packet
    return ''.join([prefix, suffix])[::-1]

def makeFCS(data):
    '''
    Do a CRC-CCITT Kermit 16bit on the data given
    Implemented using pseudocode from: June 1986, Kermit Protocol Manual
    See also: http://regregex.bbcmicro.net/crc-catalogue.htm#crc.cat.kermit

    @return: a CRC that is the FCS for the frame, as two hex bytes in
        little-endian order.
    '''
    crc = 0
    for i in xrange(len(data)):
        c = ord(data[i])
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


def pyusb_1x_patch():
    '''Monkey-patch pyusb 1.x for API compatibility
    '''

    '''
    In pyusb v1.0.0b2 (git dac78933), they removed the "length" parameter
    to usb.util.get_string(). We'll monkey-patch older versions so we don't
    have to ever pass this argument.
    '''
    if 'length' in inspect.getargspec(usb.util.get_string).args:
        print 'Monkey-patching usb.util.get_string()'
        def get_string(dev, index, langid = None):
            return usb.util.zzz__get_string(dev, 255, index, langid)
        usb.util.zzz__get_string = usb.util.get_string
        usb.util.get_string = get_string

if USBVER == 1:
    pyusb_1x_patch()
