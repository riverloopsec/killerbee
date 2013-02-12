# Import USB support depending on version of pyUSB
try:
    import usb.core
    import usb.util
    USBVER=1
    print("Warning: You are using pyUSB 1.x, support is in alpha.")
except ImportError:
    import usb
    #print("Warning: You are using pyUSB 0.x, future deprecation planned.")
    USBVER=0

import time
import struct
from datetime import datetime
from kbutils import KBCapabilities

# Functions for RZUSBSTICK, not all are implemented in firmware
# Functions not used are commented out but retained for prosperity
#RZ_CMD_SIGN_OFF             = 0x00
#RZ_CMD_SIGN_ON              = 0x01
#RZ_CMD_GET_PARAMETER        = 0x02
#RZ_CMD_SET_PARAMETER        = 0x03
#RZ_CMD_SELF_TEST            = 0x04
#RZ_CMD_CHECK_STACK_USAGE    = 0x05
#RZ_CMD_MEMORY_TEST          = 0x06
RZ_CMD_SET_MODE             = 0x07  #: RZUSB opcode to specify operating mode
RZ_CMD_SET_CHANNEL          = 0x08  #: RZUSB opcode to specify the channel
RZ_CMD_OPEN_STREAM          = 0x09  #: RZUSB opcode to open a stream for packet injection
RZ_CMD_CLOSE_STREAM         = 0x0A  #: RZUSB opcode to close a stream for packet injection
#RZ_CMD_CHANNEL_SCAN         = 0x0B
#RZ_CMD_CHANNEL_SCAN_STOP    = 0x0C
RZ_CMD_INJECT_FRAME         = 0x0D  #: RZUSB opcode to specify a frame to inject
RZ_CMD_JAMMER_ON            = 0x0E  #: RZUSB opcode to turn the jammer function on
RZ_CMD_JAMMER_OFF           = 0x0F  #: RZUSB opcode to turn the jammer function off

# Operating modes following RZ_CMD_SET_MODE function
RZ_CMD_MODE_AC              = 0x00  #: RZUSB mode for aircapture (inject + sniff)
#RZ_CMD_MODE_HAL             = 0x01
#RZ_CMD_MODE_MAC             = 0x02
#RZ_CMD_MODE_NWK             = 0x03
RZ_CMD_MODE_NONE            = 0x04 #: RZUSB no mode specified

RZ_RESP_LOCAL_TIMEOUT       = 0x00 #: RZUSB Response: Local Timeout Error
RZ_RESP_SUCCESS             = 0x80 #: RZUSB Response: Success
RZ_RESP_SYNTACTICAL_ERROR   = 0x81 #: RZUSB Response: Syntactical Error
RZ_RESP_SEMANTICAL_ERROR    = 0x82 #: RZUSB Response: Semantical Error
RZ_RESP_HW_TIMEOUT          = 0x83 #: RZUSB Response: Hardware Timeout
RZ_RESP_SIGN_ON             = 0x84 #: RZUSB Response: Sign On
RZ_RESP_GET_PARAMETER       = 0x85 #: RZUSB Response: Get Parameter
RZ_RESP_TRX_READ_REGISTER   = 0x86 #: RZUSB Response: Transceiver Read Register Error
RZ_RESP_TRX_READ_FRAME      = 0x87 #: RZUSB Response: Transceiver Read Frame Error
RZ_RESP_TRX_READ_SRAM       = 0x88 #: RZUSB Response: Transceiver Read SRAM Error
RZ_RESP_TRX_GET_PIN         = 0x89 #: RZUSB Response: Transceiver Get PIN Error
RZ_RESP_TRX_BUSY            = 0x8A #: RZUSB Response: Transceiver Busy Error
RZ_RESP_PRITMITIVE_FAILED   = 0x8B #: RZUSB Response: Primitive Failed Error
RZ_RESP_PRITMITIVE_UNKNOWN  = 0x8C #: RZUSB Response: Primitive Unknown Error
RZ_RESP_COMMAND_UNKNOWN     = 0x8D #: RZUSB Response: Command Unknown Error
RZ_RESP_BUSY_SCANING        = 0x8E #: RZUSB Response: Busy Scanning Error
RZ_RESP_BUSY_CAPTURING      = 0x8F #: RZUSB Response: Busy Capturing Error
RZ_RESP_OUT_OF_MEMORY       = 0x90 #: RZUSB Response: Out of Memory Error 
RZ_RESP_BUSY_JAMMING        = 0x91 #: RZUSB Response: Busy Jamming Error
RZ_RESP_NOT_INITIALIZED     = 0x92 #: RZUSB Response: Not Initialized Error
RZ_RESP_NOT_IMPLEMENTED     = 0x93 #: RZUSB Response: Opcode Not Implemented Error
RZ_RESP_PRIMITIVE_FAILED    = 0x94 #: RZUSB Response: Primitive Failed Error
RZ_RESP_VRT_KERNEL_ERROR    = 0x95 #: RZUSB Response: Could not execute due to vrt_kernel_error
RZ_RESP_BOOT_PARAM          = 0x96 #: RZUSB Response: Boot Param Error

RZ_EVENT_STREAM_AC_DATA          = 0x50 #: RZUSB Event Opcode: AirCapture Data
#RZ_EVENT_SNIFFER_SCAN_COMPLETE   = 0x51 #: RZUSB Event Opcode: Sniffer Scan Complete
#RZ_EVENT_SNIFFER_ERROR           = 0x52 #: RZUSB Event Opcode: Sniffer Error
#RZ_EVENT_NWK_DATA                = 0x53 #: RZUSB Event Opcode: NWK Data
#RZ_EVENT_NWK_JOIN                = 0x54 #: RZUSB Event Opcode: NWK Join Event
#RZ_EVENT_NWK_LEAVE               = 0x55 #: RZUSB Event Opcode: NWK Leave Event


RESPONSE_MAP = {RZ_RESP_LOCAL_TIMEOUT: "Local Timeout Error",
                RZ_RESP_SUCCESS : "Success",
                RZ_RESP_SYNTACTICAL_ERROR : "Syntactical Error",
                RZ_RESP_SEMANTICAL_ERROR : "Semantical Error",
                RZ_RESP_HW_TIMEOUT : "Hardware Timeout",
                RZ_RESP_SIGN_ON : "Sign On",
                RZ_RESP_GET_PARAMETER : "Get Parameter",
                RZ_RESP_TRX_READ_REGISTER : "Transceiver Read Register",
                RZ_RESP_TRX_READ_FRAME : "Transceiver Read Frame",
                RZ_RESP_TRX_READ_SRAM : "Transceiver Read SRAM",
                RZ_RESP_TRX_GET_PIN : "Transceiver Get PIN",
                RZ_RESP_TRX_BUSY : "Transceiver Busy",
                RZ_RESP_PRITMITIVE_FAILED : "Primitive Failed",
                RZ_RESP_PRITMITIVE_UNKNOWN : "Unknown Primitive",
                RZ_RESP_COMMAND_UNKNOWN : "Unknown Command",
                RZ_RESP_BUSY_SCANING : "Busy Scanning",
                RZ_RESP_BUSY_CAPTURING : "Busy Capturing",
                RZ_RESP_OUT_OF_MEMORY : "Out of Memory",
                RZ_RESP_BUSY_JAMMING : "Busy Jamming",
                RZ_RESP_NOT_INITIALIZED : "Not Initialized",
                RZ_RESP_NOT_IMPLEMENTED : "Not Implemented by USB firmware",
                RZ_RESP_PRIMITIVE_FAILED : "Primitive Failed",
                RZ_RESP_VRT_KERNEL_ERROR : "Could not execute due to vrt_kernel_error",
                RZ_RESP_BOOT_PARAM : "Boot Param Error"} #: Dictionary of RZUSB error to strings

RZ_USB_VEND_ID                = 0x03EB #: RZUSB USB VID
RZ_USB_PROD_ID                = 0x210A #: RZUSB USB PID
RZ_USB_COMMAND_EP             = 0x02 #: RZUSB USB Command Endpoint Identifier
RZ_USB_RESPONSE_EP            = 0x84 #: RZUSB USB Response Endpoint Identifier
RZ_USB_PACKET_EP              = 0x81 #: RZUSB USB Packet Endpoint Identifier

class RZUSBSTICK:
    def __init__(self, dev, bus):
        #TODO deprecate bus param, and dev becomes a usb.core.Device object, not a string in pyUSB 1.x use
        '''
        Instantiates the KillerBee class for the RZUSBSTICK hardware.

        @type dev:   TODO
        @param dev:  USB device identifier
        @type bus:   TODO
        @param bus:  Identifies the USB bus the device is on
        @return: None
        @rtype: None
        '''
        self._channel = None
        self.handle = None
        self.dev = dev
        self.__bus = bus

        if self.dev != None:
            self.__handle_open()
        else:
            raise Exception('No interface found')

        # Tracking the command operating mode (None or AirCapture Mode)
        self.__cmdmode = RZ_CMD_MODE_NONE

        # Tracking if the RZ_CMD_OPEN_STREAM parameter is set for packet reception
        self.__stream_open = False

        # Capabilities list
        self.capabilities = KBCapabilities()
        self.__set_capabilities()

    def __handle_open(self):
        '''
        Opens the device identified as self.dev, populating self.handle.
        An RZUSBSTICK has a hierarchy of:
            Config value: 1
                Interface number 0, with alternate setting 0
		            Endpoint 132 for responses
		            Endpoint 2   for control
		            Endpoint 129 for packets
        '''
        if USBVER == 0:
            self.__handle_open_v0x()
        else: #pyUSB 1.x
            self.__dev_setup_v1x()

    def __dev_setup_v1x(self):
            # See http://pyusb.sourceforge.net/docs/1.0/tutorial.html for reference
            self.dev.set_configuration()
            
            #self.dev.reset()
            #self.dev.set_configuration(1) # could also provide no config number
            #self.dev.set_interface_altsetting(interface = 0, alternate_setting = 0)
            #TODO alternative setup code:
            #self.dev.set_configuration()
            #cfg = self.dev.get_active_configuration()
            #interface_number = cfg[(0,0)].bInterfaceNumber
            #alternate_settting = usb.control.get_interface(interface_number)
            #intf = usb.util.find_descriptor(cfg, bInterfaceNumber=interface_number, bAlternateSetting=alternate_setting)
            #self.handle = usb.util.find_descriptor(intf, custom_match=lambda e: (e.bEndpointAddress == TODO_TARGET_ENDPOINT))

    def __handle_open_v0x(self):
        try:
            config = self.dev.configurations[0]
            intf = config.interfaces[0]
            alt = intf[0]
            self.handle = self.dev.open()
            self.handle.reset()
            self.handle.setConfiguration(config)
            self.handle.claimInterface(alt)
            self.handle.setAltInterface(alt)
        except:
            raise Exception("Unable to open device. " +
                            "Ensure the device is free and plugged-in. You may need sudo.")

    def close(self):
        '''
        Closes the device handle.  To be re-used, class should be re-instantiated.
        @return: None
        @rtype: None
        '''
        if USBVER == 0:
            self.handle.releaseInterface()

    # KillerBee implements these, maybe it shouldn't and instead leave it to the driver as needed.
    def check_capability(self, capab):
        return self.capabilities.check(capab)
    def get_capabilities(self):
        return self.capabilities.getlist()

    def __set_capabilities(self):
        '''
        Sets the capability information for RZUSB devices.
        @rtype: None
        @return: None
        '''
        # Examine the product string for this device, setting the capability information appropriately.
        prod = self.get_dev_info()[1] # returns a list, with second element being product string

        if prod == "RZUSBSTICK":
            self.capabilities.setcapab(KBCapabilities.SNIFF, True)
            self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        elif prod == "KILLERB001":
            self.capabilities.setcapab(KBCapabilities.SNIFF, True)
            self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
            self.capabilities.setcapab(KBCapabilities.INJECT, True)
        else:
            pass

    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device identifier,
        product string and serial number in a list of strings.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        if USBVER == 0:
            return [''.join([self.__bus.dirname, ":", self.dev.filename]), self.dev.open().getString(self.dev.iProduct, 50), self.dev.open().getString(self.dev.iSerialNumber, 12)]
        elif USBVER == 1:
            return ["{0}:{1}".format(self.dev.bus, self.dev.address),         \
                    usb.util.get_string(self.dev, 50, self.dev.iProduct),     \
                    usb.util.get_string(self.dev, 50, self.dev.iSerialNumber) ]

    def __usb_write(self, endpoint, data):
        '''
        Write data to the USB device opened as self.handle.
        
        @type endpoint: Integer
        @param endpoint: The USB endpoint to write to
        @type data: Mixed
        @param data: The data to send to the USB endpoint
        '''
        if USBVER == 0:
            try:
                self.handle.bulkWrite(endpoint, data)
                # Returns a tuple, first value is an int as the RZ_RESP_* code
                response = self.handle.bulkRead(RZ_USB_RESPONSE_EP, 1)[0]
            except usb.USBError, e:
                if e.args != ('No error',): # http://bugs.debian.org/476796
                    raise e
            time.sleep(0.0005)
            if response != RZ_RESP_SUCCESS:
                if response in RESPONSE_MAP:
                    raise Exception("Error: %s" % RESPONSE_MAP[response])
                else:
                    raise Exception("Unknown USB write error: 0x%02x" % response)
        else: #pyUSB 1.x
            res = self.dev.write(endpoint, data)#, 0, 100)
            if len(data) != res:
                raise Exception("Issue writing USB data {0} to endpoint {1}, got a return of {2}.".format(data, endpoint, res))
            try:
                response = self.dev.read(RZ_USB_RESPONSE_EP, self.dev.bMaxPacketSize0, 0, 500)
                response = response.pop()
            except usb.core.USBError as e:
                if e.errno != 110: #Not Operation timed out
                    print "Error args:", e.args
                    raise e
                elif e.errno == 110:
                    print "DEBUG: Received operation timed out error ...attempting to continue."
            time.sleep(0.0005)
            if response != RZ_RESP_SUCCESS:
                if response in RESPONSE_MAP:
                    raise Exception("Error: %s" % RESPONSE_MAP[response])
                else:
                    raise Exception("Unknown USB write error: 0x%02x" % response)

    def _set_mode(self, mode=RZ_CMD_MODE_AC):
        '''
        Change the operating mode of the USB device to one of the RZ_CMD_MODE_*
        values.  Currently, RZ_CMD_MODE_AC (Air Capture) is the only mode that is
        used other than RZ_CMD_MODE_NONE.
        @type mode: Integer
        @param mode: Operating mode for the USB stick
        @rtype: None
        '''
        self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_SET_MODE, mode])
        self.__cmdmode = mode

    def _open_stream(self):
        '''
        Opens a data stream for receiving packets.
        '''
        self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_OPEN_STREAM])
        self.__stream_open = True

    def _close_stream(self):
        '''
        Closes a data stream.
        '''
        self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_CLOSE_STREAM])
        self.__stream_open = False

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        if self.__cmdmode != RZ_CMD_MODE_AC:
            self._set_mode(RZ_CMD_MODE_AC)

        if channel != None:
            self.set_channel(channel)

        self._open_stream()

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        self._close_stream()

    def jammer_on(self, channel=None):
        '''
        Not yet implemented.  Stay tuned.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.PHYJAM)

        if self.__cmdmode != RZ_CMD_MODE_AC:
            self._set_mode(RZ_CMD_MODE_AC)

        if channel != None:
            self.set_channel(channel)

        self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_JAMMER_ON])

    def jammer_off(self, channel=None):
        '''
        Not yet implemented.  Stay tuned.
        @return: None
        @rtype: None
        '''
        self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_JAMMER_OFF])

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel):
        '''
        Sets the radio interface to the specifid channel.  Currently, support is
        limited to 2.4 GHz channels 11 - 26.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        if self.__cmdmode != RZ_CMD_MODE_AC:
            self._set_mode(RZ_CMD_MODE_AC)

        if 10 <= channel <= 26:
            self._channel = channel #update driver's notion of current channel
            self.__usb_write(RZ_USB_COMMAND_EP, [RZ_CMD_SET_CHANNEL, channel])
        else:
            raise Exception('Invalid channel')

    # KillerBee expects the driver to implement this function
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
        self.capabilities.require(KBCapabilities.INJECT)

        if self.__cmdmode != RZ_CMD_MODE_AC:
            self._set_mode(RZ_CMD_MODE_AC)

        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel != None:
            self.set_channel(channel)

        # Append two bytes to be replaced with FCS by firmware.
        packet += "\x00\x00"

        for pnum in xrange(count):
            # Format for packet is opcode RZ_CMD_INJECT_FRAME, one-byte length, 
            # packet data
            self.__usb_write(RZ_USB_COMMAND_EP, struct.pack("BB", RZ_CMD_INJECT_FRAME, len(packet)) + packet)
            time.sleep(delay)

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout=100):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if self.__stream_open == False:
            # Turn the sniffer on
            self.sniffer_on()

        # The RZ_USB_PACKET_EP doesn't return error codes like the standard
        # RZ_USB_RESPONSE_EP does, so we don't use __usb_read() here.
        pdata = None
        if USBVER == 0:
            try:
                pdata = self.handle.bulkRead(RZ_USB_PACKET_EP, timeout)
            except usb.USBError, e:
                if e.args != ('No error',): # http://bugs.debian.org/476796
                    if e.args[0] != "Connection timed out": # USB timeout issue
                        print "Error args:", e.args
                        raise e
        else: # pyUSB 1.x
            try:
                pdata = self.dev.read(RZ_USB_PACKET_EP, self.dev.bMaxPacketSize0)#1, 0, 100)
            except usb.core.USBError as e:
                if e.errno != 110: #Operation timed out
                    print "Error args:", e.args
                    raise e
            #TODO error handling enhancements for USB 1.0

        # PyUSB returns an empty tuple occasionally, handle as "no data"
        if pdata == None or pdata == ():
            return None

        if pdata[0] == RZ_EVENT_STREAM_AC_DATA:
            rssi = int(pdata[6])
            validcrc = True if (pdata[7] == 1) else False
            frame = pdata[9:]
            # Convert the tuple response data to a string (by way of a list)
            framedata = []
            for byteval in frame:
                framedata.append(struct.pack("B", byteval))
            # Bugfix rmspeers 12.17.2010
            # Remove the junk \xff byte at the end, instead of overwriting it in zbdump with a \x00 byte
            framedata = ''.join(framedata[:-1])
            #Return in a nicer dictionary format, so we don't have to reference by number indicies.
            #Note that 0,1,2 indicies inserted twice for backwards compatibility.
            return {0:framedata, 1:validcrc, 2:rssi, \
                    'bytes':framedata, 'validcrc':validcrc, 'rssi':rssi, \
                    'dbm':rssi,'datetime':datetime.now()}
            #TODO calculate dbm based on RSSI conversion formula for the chip
        else:
            return None
            #raise Exception("Unrecognized AirCapture Data Response: 0x%02x" % pdata[0])
 
    def ping(self, da, panid, sa, channel=None):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        raise Exception('Not yet implemented')

