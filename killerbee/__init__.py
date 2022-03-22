from typing import Optional, Any, Dict, Union, List

import struct
import glob
from warnings import warn

from .pcapdump import *
from .daintree import *
from .pcapdlt import *

from .kbutils import devlist
from .kbutils import isIpAddr
from .kbutils import search_usb
from .kbutils import isSerialDeviceString
from .kbutils import issl_nodetest
from .kbutils import issl_beehive
from .kbutils import iszigduino
from .kbutils import isfreakduino
from .kbutils import *
from .zigbeedecode import * #would like to import only within killerbee class
from .dot154decode import * #would like to import only within killerbee class
from .config import *       #to get DEV_ENABLE_* variables

# Utility Functions
def show_dev(vendor: str=None, product: str=None, gps: str=None, include: str=None) -> None:
    '''
    A basic function to output the device listing.
    Placed here for reuse, as many tool scripts were implementing it.
    @param gps: Provide device names in this argument (previously known as
        'gps') which you wish to not be enumerated. Aka, exclude these items.
    @param include: Provide device names in this argument if you would like only
        these to be enumerated. Aka, include only these items.
    '''
    fmt: str = "{: >14} {: <30} {: >10}"
    print((fmt.format("Dev", "Product String", "Serial Number")))
    for dev in devlist(vendor=vendor, product=product, gps=gps, include=include): 
        # Using None as a format value is an TypeError in python3
        print((fmt.format(dev[0], dev[1], str(dev[2]))))

# KillerBee Class
class KillerBee:
    def __init__(self, device: Optional[str]=None, hardware: Optional[str]=None, datasource: Optional[str]=None, gps: Optional[str]=None) -> None:
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
        self.dev: Optional[Any] = None
        self.__bus: Optional[Any] = None
        self.driver: Optional[Any] = None

        #TODO deprecate
        global gps_devstring
        if gps_devstring is None and gps is not None:
            gps_devstring = gps

        if hardware is not None and device is not None:
            if hardware == "apimote":
                from .dev_apimote import APIMOTE
                self.driver = APIMOTE(device)
            elif hardware == "rzusbstick":
                from .dev_rzusbstick import RZUSBSTICK
                self.driver = RZUSBSTICK(device, self.__bus)
            elif hardware == "cc2530":
                from .dev_cc253x import CC253x
                self.driver = CC253x(device, self.__bus, CC253x.VARIANT_CC2530)
            elif hardware == "cc2531":
                from .dev_cc253x import CC253x
                self.driver = CC253x(device, self.__bus, CC253x.VARIANT_CC2531)
            elif hardware == "bumblebee":
                from .dev_bumblebee import Bumblebee
                self.driver = Bumblebee(device, self.__bus)
            elif hardware == "sl_nodetest":
                from .dev_sl_nodetest import SL_NODETEST
                self.driver = SL_NODETEST(device)
            elif hardware == "sl_beehive":
                from .dev_sl_beehive import SL_BEEHIVE
                self.driver = SL_BEEHIVE(device)
            elif hardware == "zigduino":
                from .dev_zigduino import ZIGDUINO
                self.driver = ZIGDUINO(device)
            elif hardware == "freakdruino":
                from .dev_freakduino import FREAKDUINO
                self.driver = FREAKDUINO(device)
            elif hardware == "telosb":
                from .dev_telosb import TELOSB
                self.driver = TELOSB(device)
            elif hardware == "sewio":
                from .dev_sewio import SEWIO
                self.driver = SEWIO(dev=device)

        else:
            if self.driver is None:
                # Check if IP device
                if device is not None and isIpAddr(device):
                    from .dev_sewio import isSewio
                    if isSewio(device):
                        from .dev_sewio import SEWIO
                        self.driver = SEWIO(dev=device)  # give it the ip address
                    else: del isSewio

                # Check if USB device
                if device is None :
                    result = search_usb(None)
                    if result is not None:
                        self.dev = result
                elif ":" in device:
                    result = search_usb(None)
                    if result is not None:
                        self.dev = result
                    else:
                        raise KBInterfaceError("Did not find a USB device matching %s." % device)

                # If USB, identify hardware
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
                    elif self.__device_is(BB_USB_VEND_ID, BB_USB_PROD_ID):
                        from .dev_bumblebee import Bumblebee
                        self.driver = Bumblebee(self.dev, self.__bus)
                    else:
                        raise KBInterfaceError("KillerBee doesn't know how to interact with USB device vendor=%04x, product=%04x." % (self.dev.idVendor, self.dev.idProduct))

            if self.driver is None:
                # Check if Serial device
                if device is None:
                    glob_list = get_serial_ports()
                    if len(glob_list) > 0:
                        result = glob_list[0]
                        if isSerialDeviceString(result):
                          self.dev = result
                        else:
                          raise KBInterfaceError("KillerBee doesn't understand device given by '%s'." % device)
                elif isSerialDeviceString(device):
                    self.dev = device

                # If Serial, identify hardware
                if self.dev is not None:
                    if (self.dev == gps_devstring):
                        pass
                    elif (DEV_ENABLE_SL_NODETEST and issl_nodetest(self.dev)):
                        from .dev_sl_nodetest import SL_NODETEST
                        self.driver = SL_NODETEST(self.dev)
                    elif (DEV_ENABLE_SL_BEEHIVE and issl_beehive(self.dev)):
                        from .dev_sl_beehive import SL_BEEHIVE
                        self.driver = SL_BEEHIVE(self.dev)
                    elif (DEV_ENABLE_ZIGDUINO and iszigduino(self.dev)):
                        from .dev_zigduino import ZIGDUINO
                        self.driver = ZIGDUINO(self.dev)
                    elif (DEV_ENABLE_FREAKDUINO and isfreakduino(self.dev)):
                        from .dev_freakduino import FREAKDUINO
                        self.driver = FREAKDUINO(self.dev)
                    else:
                        gfccspi,subtype = isgoodfetccspi(self.dev)
                        if gfccspi and subtype == 0:
                            from .dev_telosb import TELOSB
                            self.driver = TELOSB(self.dev)
                        elif gfccspi and subtype == 2:
                            from .dev_apimote import APIMOTE
                            self.driver = APIMOTE(self.dev)
                        else:
                            raise KBInterfaceError("KillerBee doesn't know how to interact with serial device at '%s'." % self.dev)

        if self.driver is None:
            raise KBInterfaceError("KillerBee cannot find device.")

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
        if self.dev.idVendor == vendorId and self.dev.idProduct == productId: 
            return True
        else: 
            return False

    def get_dev_info(self) -> List[str]:
        '''
        Returns device information in a list identifying the device. Implemented by the loaded driver.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.get_dev_info()

    def close(self) -> None:
        '''
        Closes the device out.
        @return: None
        @rtype: None
        '''
        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        else:
            self.driver.close()

        if hasattr(self, "dblog") and (self.dblog is not None):
            self.dblog.close()

    def check_capability(self, capab: str) -> bool:
        '''
        Uses the specified capability to determine if the opened device
        is supported.  Returns True when supported, else False.
        @rtype: Boolean
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.capabilities.check(capab)

    def is_valid_channel(self, channel: int, page: int=0) -> bool:
        '''
        Use the driver's capabilities class to determine if a requested channel number
        is within the capabilities of that device.
        @rtype: Boolean
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.capabilities.is_valid_channel(channel, page)

    def frequency(self, channel: Optional[int]=None, page: int=0) -> int:
        '''
        Use the driver's capabilities class to convert channel and page to actual frequency in KHz
        @rtype: Integer
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.capabilities.frequency(channel, page)

    def get_capabilities(self) -> List[Any]:
        '''
        Returns a list of capability information for the device.
        @rtype:  List
        @return: Capability information for the opened device.
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.capabilities.getlist()

    def enter_bootloader(self) -> Any:
        '''
        Starts the bootloader
        @rtype: None
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.enter_bootloader()

    def get_bootloader_version(self) -> List[int]:
        '''
        Gets the bootloader major and minor version.
        @rtype:  List
        @return: Returns a list: [Major, Minor]
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.get_bootloader_version()

    def get_bootloader_signature(self) -> List[int]:
        '''
        Gets the bootloader chip signature.
        @rtype:  List
        @return: Returns a list: [Low, Mid, High]
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.get_bootloader_signature()

    def bootloader_sign_on(self) -> str:
        '''
        @rtype: String
        @return: Bootloader sign_on message
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.bootloader_sign_on()

    def bootloader_start_application(self) -> Any:
        '''
        Instructs the bootloader to exit and run the app
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.bootloader_start_application()

    def sniffer_on(self, channel: Optional[int]=None, page: int=0) -> Any:
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.sniffer_on(channel, page)

    def sniffer_off(self) -> Any:
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.sniffer_off()

    @property
    def channel(self) -> int:
        """Getter function for the channel that was last set on the device."""

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        # Driver must have this variable name set in its set_channel function
        return self.driver._channel

    @property
    def page(self) -> int:
        """Getter function for the page that was last set on the device."""

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        # Driver must have this variable name set in its set_channel function
        return self.driver._page

    def set_channel(self, channel: int, page: int=0) -> None:
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
        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        if hasattr(self, "dblog"):
            self.dblog.set_channel(channel, page)

        self.driver.set_channel(channel, page)

    def inject(self, packet: bytes, channel: Optional[int]=None, count: int=1, delay: int=0, page: int=0) -> Any:
        '''
        Injects the specified packet contents.
        @type packet: Bytes 
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

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.inject(packet, channel, count, delay, page)

    def pnext(self, timeout: int=100) -> Optional[Dict[Union[int, str], Any]]:
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned { 0: (str) packet contents | 1: (bool) Valid CRC | 2: (int) Unscaled RSSI }
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.pnext(timeout)

    def jammer_on(self, channel: Optional[int]=None, method: Optional[str]=None):
        '''
        Attempts reflexive jamming on all 802.15.4 frames.
        Targeted frames must be >12 bytes for reliable jamming in current firmware.
        @type channel: Integer
        @param channel: Sets the channel, optional.
        @rtype: None
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.jammer_on(channel=channel, method=method)


    def jammer_off(self):
        '''
        End reflexive jamming on all 802.15.4 frames.
        Targeted frames must be >12 bytes for reliable jamming in current firmware.
        @type channel: Integer
        @param channel: Sets the channel, optional.
        @rtype: None
        '''

        if self.driver is None:
            raise KBInterfaceError("Driver not configured")

        return self.driver.jammer_off()

