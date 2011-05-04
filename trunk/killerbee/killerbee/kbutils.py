import usb, serial
import os, glob
import random
from struct import pack

RZ_USB_VEND_ID      = 0x03EB
RZ_USB_PROD_ID      = 0x210A
ZN_USB_VEND_ID      = 0x04D8
ZN_USB_PROD_ID      = 0x000E
usbVendorList = [RZ_USB_VEND_ID, ZN_USB_VEND_ID]
usbProductList = [RZ_USB_PROD_ID, ZN_USB_PROD_ID]

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
    def __init__(self):
        self._capabilities = {
                self.NONE : False,
                self.SNIFF : False,
                self.SETCHAN : False,
                self.INJECT : False,
                self.PHYJAM : False,
                self.SELFACK: False}
    def check(self, capab):
        try:
            return self._capabilities[capab]
        except KeyError:
            return False
    def getlist(self):
        return self._capabilties
    def setcapab(self, capab, value):
        self._capabilities[capab] = value
    def require(self, capab):
        if self.check(capab) != True:
            raise Exception('Selected hardware does not support required capability (%d).' % capab)
            
def devfind(vendor=None, product=None):
    '''
    Return device information for all present devices.
    @rtype: List
    @return: List of device information present.
    '''
    global usbVendorList, usbProductList
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

    seriallist = glob.glob("/dev/ttyUSB*") #TODO make cross platform globing/winnt
    #print "Serial List:", seriallist
    for serialdev in seriallist:
        if (isgoodfet(serialdev)):
            devlist.append([serialdev, "telosb/tmote", ""])
        elif (isfreakduino(serialdev)):
            devlist.append([serialdev, "Dartmouth Freakduino", ""])
    return devlist
    
def isgoodfet(serialdev):
    from GoodFETCCSPI import GoodFETCCSPI
    #TODO make so doesn't need local copies of GoodFET[CCSPI] code files
    os.environ["platform"] = "telosb" #set enviroment variable for GoodFET code to use
    gf = GoodFETCCSPI()
    gf.serInit(port=serialdev, attemptlimit=2)
    return (gf.connected == 1)
    
def isfreakduino(serialdev):
    s = serial.Serial(port=serialdev, baudrate=57600, timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
    time.sleep(1.5)
    s.write('SC!V\r')
    time.sleep(1.5)
    s.readline(eol='&')
    if s.read(3) == 'C!V': version = s.read()
    else: version = None
    s.close()
    return (version is not None)

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
    Returns a random MAC address using a list valid OUI's from ZigBee device manufacturers.  Data is returned in air-format byte order (LSB first).
    @type length: String
    @param length: Optional length of MAC address, def=8.  Minimum address return length is 3 bytes for the valid OUI.
    @rtype: String
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

    prefix = prefixes[random.randrange(0, len(prefixes))]
    suffix = randbytes(length-3)
    # Reverse the address for use in a packet
    return ''.join([prefix, suffix])[::-1]

# Do a CRC-CCITT Kermit 16bit on the data given
# Returns a CRC that is the FCS for the frame
#  Implemented using pseudocode from: June 1986, Kermit Protocol Manual
#  See also: http://regregex.bbcmicro.net/crc-catalogue.htm#crc.cat.kermit
def makeFCS(data):
    crc = 0
    for i in range(0, len(data)):
        c = ord(data[i])
		#if (A PARITY BIT EXISTS): c = c & 127	#Mask off any parity bit
        q = (crc ^ c) & 15				#Do low-order 4 bits
        crc = (crc // 16) ^ (q * 4225)
        q = (crc ^ (c // 16)) & 15		#And high 4 bits
        crc = (crc // 16) ^ (q * 4225)
    return pack('<H', crc) #return as bytes in little endian order
