import struct
import glob
from warnings import warn

from .pcapdump import *
from .daintree import *
from .pcapdlt import *

from .kbutils import *      #provides serial, usb, USBVER
from .zigbeedecode import * #would like to import only within killerbee class
from .dot154decode import * #would like to import only within killerbee class
from .config import *       #to get DEV_ENABLE_* variables

# Utility Functions
def getKillerBee(channel, page=0):
    '''
    Returns an instance of a KillerBee device, setup on the given channel/page.
    Error handling for KillerBee creation and setting of the channel is wrapped
    and will raise an Exception().
    @return: A KillerBee instance initialized to the given channel/page.
    '''
    kb = KillerBee()
    if kb is None:
        raise Exception("Failed to create a KillerBee instance.")
    try:
        kb.set_channel(channel, page)
    except Exception as e:
        raise Exception('Error: Failed to set channel to %d/%d' % (channel, page), e)
    return kb

def kb_dev_list(vendor=None, product=None, gps=None, include=None):
    '''Deprecated. Use show_dev or call kbutils.devlist.'''
    return kbutils.devlist(vendor=vendor, product=product, gps=gps, include=include)

def show_dev(vendor=None, product=None, gps=None, include=None):
    '''
    A basic function to output the device listing.
    Placed here for reuse, as many tool scripts were implementing it.
    @param gps: Provide device names in this argument (previously known as
        'gps') which you wish to not be enumerated. Aka, exclude these items.
    @param include: Provide device names in this argument if you would like only
        these to be enumerated. Aka, include only these items.
    '''
    fmt_str = "{: >14} {: <25} {: >10}"
    print(fmt_str.format("Dev", "Product String", "Serial Number"))
    for dev in kbutils.devlist(vendor=vendor, product=product, gps=gps, include=include):
        print(fmt_str.format(dev[0], dev[1], dev[2] if dev[2] is not None else ""))

# KillerBee Class
class KillerBee:
    def __init__(self, device=None, datasource=None, gps=None):
        '''
        Instantiates the KillerBee class.

        @type device:   String
        @param device:  Device identifier, which is either USB `<BusNumber>:<DeviceNumber>`,
            serial device path (e.g., `/dev/ttyUSB0`), or IP address.
            The format needed depends on the device's firmware and connectivity to the host system.
        @type datasource: String
        @param datasource: A known data-source type that is used by dblog to record how the data was captured.
        @type gps: String
        @param gps: Optional serial device identifier for an attached GPS unit.
            If provided, or if global variable has previously been set,
            KillerBee skips that device in initialization process.
        @return: None
        @rtype: None
        '''

        global gps_devstring
        if gps_devstring is None and gps is not None:
            gps_devstring = gps

        self.dev = None
        self.__bus = None
        self.driver = None

        # IP devices may be the most straightforward, and we aren't doing
        # discovery, just connecting to defined addresses, so we'll check
        # first to see if we have an IP address given as our device parameter.
        if (device is not None) and kbutils.isIpAddr(device):
            from .dev_sewio import isSewio
            if isSewio(device):
                from .dev_sewio import SEWIO
                self.driver = SEWIO(dev=device)  # give it the ip address
            else: del isSewio

        # Figure out a device is one is not set, trying USB devices next
        if self.driver is None:
            if device is None:
                result = kbutils.search_usb(None)
                if result != None:
                    if USBVER == 0:
                        (self.__bus, self.dev) = result
                    elif USBVER == 1:
                        #TODO remove self.__bus attribute, not needed in 1.x as all info in self.dev
                        self.dev = result
            # Recognize if device is provided in the USB format (like a 012:456 string):
            elif ":" in device:
                result = kbutils.search_usb(device)
                if result == None:
                    raise KBInterfaceError("Did not find a USB device matching %s." % device)
                else:
                    if USBVER == 0:
                        (self.__bus, self.dev) = result
                    elif USBVER == 1:
                        #TODO remove self.__bus attribute, not needed in 1.x as all info in self.dev
                        self.dev = result

            if self.dev is not None:
                if self.__device_is(RZ_USB_VEND_ID, RZ_USB_PROD_ID):
                    from .dev_rzusbstick import RZUSBSTICK
                    self.driver = RZUSBSTICK(self.dev, self.__bus)
                elif self.__device_is(ZN_USB_VEND_ID, ZN_USB_PROD_ID):
                    raise KBInterfaceError("Zena firmware not yet implemented.")
                elif self.__device_is(CC2530_USB_VEND_ID, CC2530_USB_PROD_ID):
                    from .dev_cc253x import CC253x
                    self.driver = CC253x(self.dev, self.__bus, CC253x.VARIANT_CC2530)
                elif self.__device_is(CC2531_USB_VEND_ID, CC2531_USB_PROD_ID):
                    from .dev_cc253x import CC253x
                    self.driver = CC253x(self.dev, self.__bus, CC253x.VARIANT_CC2531)
                else:
                    raise KBInterfaceError("KillerBee doesn't know how to interact with USB device vendor=%04x, product=%04x." % (self.dev.idVendor, self.dev.idProduct))

        # Figure out a device from serial if one is not set
        #TODO be able to try more than one serial device here (merge with devlist code somehow)
#        if device == None:
#            seriallist = get_serial_ports()
#            if len(seriallist) > 0:
#                device = seriallist[0]

        # If a USB device driver was not loaded, now we try serial devices
        if self.driver is None:
            # If no device was specified
            if device is None:
                glob_list = get_serial_ports()
                if len(glob_list) > 0:
                    #TODO be able to check other devices if this one is not correct
                    device = glob_list[0]
            # Recognize if device specified by serial string:
            if (device is not None) and kbutils.isSerialDeviceString(device):
                self.dev = device
                if (self.dev == gps_devstring):
                    pass
                elif (DEV_ENABLE_SL_NODETEST and kbutils.issl_nodetest(self.dev)):
                    from .dev_sl_nodetest import SL_NODETEST
                    self.driver = SL_NODETEST(self.dev)
                elif (DEV_ENABLE_SL_BEEHIVE and kbutils.issl_beehive(self.dev)):
                    from .dev_sl_beehive import SL_BEEHIVE
                    self.driver = SL_BEEHIVE(self.dev)
                elif (DEV_ENABLE_ZIGDUINO and kbutils.iszigduino(self.dev)):
                    from .dev_zigduino import ZIGDUINO
                    self.driver = ZIGDUINO(self.dev)
                elif (DEV_ENABLE_FREAKDUINO and kbutils.isfreakduino(self.dev)):
                    from .dev_freakduino import FREAKDUINO
                    self.driver = FREAKDUINO(self.dev)
                else:
                    gfccspi,subtype = isgoodfetccspi(self.dev)
                    if gfccspi and subtype == 0:
                        from .dev_telosb import TELOSB
                        self.driver = TELOSB(self.dev)
                    elif gfccspi and subtype == 1:
                        from .dev_apimote import APIMOTE
                        self.driver = APIMOTE(self.dev, revision=1)
                    elif gfccspi and subtype == 2:
                        from .dev_apimote import APIMOTE
                        self.driver = APIMOTE(self.dev, revision=2)
                    else:
                        raise KBInterfaceError("KillerBee doesn't know how to interact with serial device at '%s'." % self.dev)
            # Otherwise unrecognized device string type was provided:
            else:
                raise KBInterfaceError("KillerBee doesn't understand device given by '%s'." % device)

        # Start a connection to the remote packet logging server, if able:
        if datasource is not None:
            try:
                from . import dblog
                self.dblog = dblog.DBLogger(datasource)
            except Exception as e:
                warn("Error initializing DBLogger (%s)." % e)
                datasource = None   #give up nicely if error connecting, etc.

    # Allow 'with KillerBee(...) as kb:' syntax
    def __enter__(self):
        return self

    def __exit__(self, *exinfo):
        self.close()

    def __device_is(self, vendorId, productId):
        '''
        Compares KillerBee class' device data to a known USB vendorId and productId
        @type vendorId: 
        @type productId: 
        @rtype: Boolean
        @return: True if KillerBee class has device matching the vendor and product IDs provided.
        '''
        if self.dev.idVendor == vendorId and self.dev.idProduct == productId: return True
        else: return False

    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device. Implemented by the loaded driver.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return self.driver.get_dev_info()

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
        @rtype: Boolean
        '''
        return self.driver.capabilities.check(capab)

    def is_valid_channel(self, channel, page=0):
        '''
        Use the driver's capabilities class to determine if a requested channel number
        is within the capabilities of that device.
        @rtype: Boolean
        '''
        return self.driver.capabilities.is_valid_channel(channel, page)

    def frequency(self, channel=None, page=0):
        '''
        Use the driver's capabilities class to convert channel and page to actual frequency in KHz
        @rtype: Integer
        '''
        return self.driver.capabilities.frequency(channel, page)

    def get_capabilities(self):
        '''
        Returns a list of capability information for the device.
        @rtype:  List
        @return: Capability information for the opened device.
        '''
        return self.driver.capabilities.getlist()

    def enter_bootloader(self):
        '''
        Starts the bootloader
        @rtype: None
        '''
        return self.driver.enter_bootloader()

    def get_bootloader_version(self):
        '''
        Gets the bootloader major and minor version.
        @rtype:  List
        @return: Returns a list: [Major, Minor]
        '''
        return self.driver.get_bootloader_version()

    def get_bootloader_signature(self):
        '''
        Gets the bootloader chip signature.
        @rtype:  List
        @return: Returns a list: [Low, Mid, High]
        '''
        return self.driver.get_bootloader_signature()

    def bootloader_sign_on(self):
        '''
        @rtype: String
        @return: Bootloader sign_on message
        '''
        return self.driver.bootloader_sign_on()

    def bootloader_start_application(self):
        '''
        Instructs the bootloader to exit and run the app
        '''

        return self.driver.bootloader_start_application()

    def sniffer_on(self, channel=None, page=0):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        return self.driver.sniffer_on(channel, page)

    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        return self.driver.sniffer_off()

    @property
    def channel(self):
        """Getter function for the channel that was last set on the device."""
        # Driver must have this variable name set in its set_channel function
        return self.driver._channel

    @property
    def page(self):
        """Getter function for the page that was last set on the device."""
        # Driver must have this variable name set in its set_channel function
        return self.driver._page

    def set_channel(self, channel, page=0):
        '''
        Sets the radio interface to the specifid channel & page (subghz)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the page, optional
        @rtype: None
        '''
        if not self.is_valid_channel(channel, page):
            raise ValueError('Invalid channel ({0}) for this device'.format(channel))
        if hasattr(self, "dblog"):
            self.dblog.set_channel(channel, page)
        self.driver.set_channel(channel, page)

    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        '''
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, without FCS.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type channel: Integer
        @param page: Sets the subghz page, optional
        @type count: Integer
        @param count: Transmits a specified number of frames, def=1
        @type delay: Float
        @param delay: Delay between each frame, def=1
        @rtype: None
        '''
        return self.driver.inject(packet, channel, count, delay, page)

    def pnext(self, timeout=100):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned { 0: (str) packet contents | 1: (bool) Valid CRC | 2: (int) Unscaled RSSI }
        '''
        return self.driver.pnext(timeout)

    def jammer_on(self, channel=None, page=0, mode=1):
        '''
        Attempts reflexive jamming on all 802.15.4 frames.
        Targeted frames must be >12 bytes for reliable jamming in current firmware.
        @type channel: Integer
        @param channel: Sets the channel, optional.
        @rtype: None
        '''
        return self.driver.jammer_on(channel=channel)

    def jammer_off(self, channel=None, page=0):
        '''
        End reflexive jamming on all 802.15.4 frames.
        Targeted frames must be >12 bytes for reliable jamming in current firmware.
        @type channel: Integer
        @param channel: Sets the channel, optional.
        @rtype: None
        '''
        return self.driver.jammer_off(channel=channel)

