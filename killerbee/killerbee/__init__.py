import usb
import serial
import struct
import os, time, glob
from pcapdump import *
from daintree import *
from pcapdlt import *
from kbutils import *
from zigbeedecode import * #would like to import only within killerbee class
from dot154decode import * #would like to import only within killerbee class

# Define what vendors and products we recognize, in order to only consider appropriate USB devices:
RZ_USB_VEND_ID      = 0x03EB
RZ_USB_PROD_ID      = 0x210A
ZN_USB_VEND_ID      = 0x04D8
ZN_USB_PROD_ID      = 0x000E
#FTDI_USB_VEND_ID      = 0x0403
#FTDI_USB_PROD_ID      = 0x6001
usbVendorList = [RZ_USB_VEND_ID, ZN_USB_VEND_ID]
usbProductList = [RZ_USB_PROD_ID, ZN_USB_PROD_ID]

# Utility Functions
def getKillerBee(channel):
	kb = KillerBee()
	if kb is None:
		raise Exception("Failed to create a KillerBee instance.")
	try:
		kb.set_channel(channel)
	except Exception, e:
		raise Exception('Error: Failed to set channel to %d' % channel, e)
	return kb
	
def devfind():
    return kbutils.devfind()

# Functions relating to locating/identifying devices
def kb_search_usb(device, vendor=None, product=None):
    busses = usb.busses()
    for bus in busses:
        dev = kb_search_usb_bus(bus, device)
        if dev != None:
            return (bus, dev)
    return (None, None)

def kb_search_usb_bus(bus, device, vendor=None, product=None):
    global vendorList, productList
    devices = bus.devices
    for dev in devices:
        if ((vendor==None and dev.idVendor in usbVendorList) or dev.idVendor==vendor) \
           and ((product==None and dev.idProduct in usbProductList) or dev.idProduct==product):
            if device == None or (device == (''.join([bus.dirname, ":", dev.filename]))):
                #Populate the capability information for this device later, when driver is initialized
                #print "Choose device", bus.dirname, dev.filename, "to initialize KillerBee instance on."
                return dev
    return None

def kb_dev_list(vendor=None, product=None):
    '''
    Return device information for all present devices, filtering if requested by vendor and/or product IDs on USB devices.
    @rtype: List
    @return: List of device information present.
                For USB devices, get [busdir:devfilename, productString, serialNumber]
                For serial devices, get [serialFileName, deviceDescription, ""]
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
        if (kb_dev_serial_is_goodfetccspi(serialdev)):
            devlist.append([serialdev, "GoodFET telosb/tmote", ""])
        elif (kb_dev_serial_is_freakduino(serialdev)):
            devlist.append([serialdev, "Dartmouth Freakduino", ""])
    return devlist

def kb_dev_serial_is_goodfetccspi(serialdev):
    '''Given a path to a serial device (ex /dev/ttyUSB0), determine if it is a TelosB/Tmote GoodFET attached.'''
    from GoodFETCCSPI import GoodFETCCSPI
    os.environ["platform"] = "telosb" #set enviroment variable for GoodFET code to use
    gf = GoodFETCCSPI()
    gf.serInit(port=serialdev, attemptlimit=2)
    status = (gf.connected == 1)
    gf.serClose()
    return status

def kb_dev_serial_is_freakduino(serialdev):
    '''Given a path to a serial device (ex /dev/ttyUSB0), determine if it is a Freakduino attached with the right sketch loaded.'''
    s = serial.Serial(port=serialdev, baudrate=57600, timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
    time.sleep(1.5)
    s.write('SC!V\r')
    time.sleep(1.5)
    #If you get "TypeError: readline() takes no keyword arguments"
    # this results from a version issue in pySerial.
    # readline should take an eol argument, per:
    # http://pyserial.sourceforge.net/pyserial_api.html#serial.FileLike.readline
    s.readline(eol='&')
    if s.read(3) == 'C!V': version = s.read()
    else: version = None
    s.close()
    return (version is not None)


# KillerBee Class
class KillerBee:
    def __init__(self, device=None, datasource=None):
        '''
        Instantiates the KillerBee class.

        @type device:   String
        @param device:  USB or serial device identifier
        @return: None
        @rtype: None
        '''

        self.dev = None
        self.__bus = None
        self.driver = None

        # Figure out a device is one is not set
        if (device is None):
            (self.__bus, self.dev) = kb_search_usb(None)
        # Recognize if device is provided in the USB format (like a 012:456 string):
        elif (len(device)>=4 and device[3] == ":"):
            (self.__bus, self.dev) = kb_search_usb(device)
            if self.dev == None:
                raise Exception("Did not find a USB device matching %s." % device)
                
        # Figure out a device from serial if one is not set
        if (device is None):
            glob_list = glob.glob("/dev/ttyUSB*");
            if len(glob_list) > 0:
                device = glob_list[0];
                
        if self.dev is not None:
            if self.__device_is(RZ_USB_VEND_ID, RZ_USB_PROD_ID):
                from dev_rzusbstick import RZUSBSTICK
                self.driver = RZUSBSTICK(self.dev, self.__bus)
            elif self.__device_is(ZN_USB_VEND_ID, ZN_USB_PROD_ID):
                raise Exception("Zena firmware not yet implemented.")
            else:
                raise Exception("KillerBee doesn't know how to interact with USB device vendor=%04x, product=%04x." \
                                % (self.dev.idVendor, self.dev.idProduct))

        # If a USB device driver was not loaded, now we try serial devices
        if (self.driver is None):
            # If no device was specified
            if (device is None):
                glob_list = glob.glob("/dev/ttyUSB*")
                if len(glob_list) > 0:
                    device = glob_list[0] #TODO verify this device is one meant for KillerBee
            # Recognize if device specified by serial string:
            if (device is not None and device[:5] == "/dev/"):
                self.dev = device #TODO validate device string is for a real port?
                if kb_dev_serial_is_freakduino(self.dev):
                    from dev_freakduino import FREAKDUINO
                    self.driver = FREAKDUINO(self.dev)
                elif kb_dev_serial_is_goodfetccspi(self.dev):
                    from dev_telosb import TELOSB
                    self.driver = TELOSB(self.dev)
                else:
                    raise Exception("KillerBee doesn't know how to interact with serial device at '%s'." % self.dev)
            # Otherwise unrecognized device string type was provided:
            else:
                raise Exception("KillerBee doesn't understand device given by '%s'." % device)

        # Start a connection to the remote packet logging server, if able:
        if datasource is not None:
            import dblog
            self.dblog = dblog.DBLogger(datasource)

    # Functions relating to locating/identifying devices
    def __device_is(self, vendorId, productId):
        if self.dev.idVendor == vendorId and self.dev.idProduct == productId: return True
        else: return False

    #Deprecated in class
    def __search_usb(self, device, vendor=None, product=None):
        raise DeprecationWarning("Use kb_search_usb(device, vendor, product) instead of class version.")
        #return kb_search_usb(device, vendor, product)

    #Deprecated in class
    def __search_usb_bus(self, bus, device, vendor=None, product=None):
        raise DeprecationWarning("Use kb_search_usb_bus(bus, device, vendor, product) instead of class version.")
        #return kb_search_usb_bus(bus, device, vendor, product)

    #Implemented by driver
    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return self.driver.get_dev_info()

    #Deprecated in class
    def dev_list(self, vendor=None, product=None):
        raise DeprecationWarning("Use kb_dev_list(vendor, product) instead of class version.")
        #return kb_dev_list(vendor, product)

    def close(self):
        '''
        Closes the device out.
        @return: None
        @rtype: None
        '''
        if self.driver != None: self.driver.close()
        if hasattr(self, "dblog") and (self.dblog is not None):
            self.dblog.close()

    def check_capability(self, capab):
        '''
        Uses the specified capability to determine if the opened device
        is supported.  Returns True when supported, else False.
        @rtype:  Boolean
        '''
        return self.driver.capabilities.check(capab)

    def get_capabilities(self):
        '''
        Returns a list of capability information for the device.
        @rtype:  List
        @return: Capability information for the opened device.
        '''
        return self.driver.capabilities.getlist()

    def sniffer_on(self, channel=None):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        return self.driver.sniffer_on(channel)

    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        return self.driver.sniffer_off()

    def set_channel(self, channel):
        '''
        Sets the radio interface to the specifid channel.  Currently, support is
        limited to 2.4 GHz channels 11 - 26.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        if hasattr(self, "dblog"):
            self.dblog.set_channel(channel)
        self.driver.set_channel(channel)

    def inject(self, packet, channel=None, count=1, delay=0):
        '''
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, without FCS.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type count: Integer
        @param count: Transmits a specified number of frames, def=1
        @type delay: Float
        @param delay: Delay between each frame, def=1
        @rtype: None
        '''
        return self.driver.inject(packet, channel, count, delay)

    def pnext(self, timeout=100):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        return self.driver.pnext(timeout)

