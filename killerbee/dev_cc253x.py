from __future__ import print_function # type: ignore
"""
CC253x support is contributed by Scytmo.
"""


import sys # type: ignore
import struct # type: ignore
import time # type: ignore
from datetime import datetime # type: ignore
from .kbutils import KBCapabilities, makeFCS, bytearray_to_bytes # type: ignore

# Import USB support depending on version of pyUSB
import usb.core # type: ignore
import usb.util # type: ignore
import sys # type: ignore

class CC253x:
    USB_DIR_OUT        = 0x40
    USB_DIR_IN         = 0xC0
    USB_POWER_ON       = 0xC5
    USB_POWER_STATUS   = 0xC6
    USB_XFER_START     = 0xD0
    USB_XFER_STOP      = 0xD1
    USB_XFER_CHAN      = 0xD2
    USB_CC2530_DATA_EP = 0x82
    USB_CC2531_DATA_EP = 0x83

    VARIANT_CC2530 = 0
    VARIANT_CC2531 = 1

    def __init__(self, dev, bus, variant):
        #TODO deprecate bus param, and dev becomes a usb.core.Device object, not a string in pyUSB 1.x use
        """
        Instantiates the KillerBee class for Zigduino running GoodFET firmware.
        @type dev:   String
        @param dev:  PyUSB device
        @return: None
        @rtype: None
        """

        if variant == CC253x.VARIANT_CC2530:
            self._data_ep = CC253x.USB_CC2530_DATA_EP
        else:
            self._data_ep = CC253x.USB_CC2531_DATA_EP
        self._channel = None
        self._page = 0
        self.dev = dev

        self.__stream_open = False
        self.capabilities = KBCapabilities()
        self.__set_capabilities()

        # Set default configuration
        self.dev.set_configuration()

        # get name from USB descriptor
        self.name = usb.util.get_string(self.dev, self.dev.iProduct)

        # Get wMaxPacketSize from the data endpoint
        for cfg in self.dev:
            for intf in cfg:
                for ep in intf:
                    if ep.bEndpointAddress == self._data_ep:
                        self._maxPacketSize = ep.wMaxPacketSize


    def close(self):
        if self.__stream_open == True:
            self.sniffer_off()
        pass

    def check_capability(self, capab):
        return self.capabilities.check(capab)

    def get_capabilities(self):
        return self.capabilities.getlist()

    def __set_capabilities(self):
        """
        Sets the capability information appropriate for CC253x.
        @rtype: None
        @return: None
        """
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)

    # KillerBee expects the driver to implement this function
    def get_dev_info(self):
        """
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        """
        # TODO Determine if there is a way to get a unique ID from the device
        return [self.name, "CC253x", ""]

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        """
        Turns the sniffer on such that pnext() will start returning observed data.
        Will set the command mode to Air Capture if it is not already set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        self.capabilities.require(KBCapabilities.SNIFF)

        if channel != None:
            self.set_channel(channel, page)

        # Enable power in 802.15.4 radio
        self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_POWER_ON, wIndex = 4)
        while True:
            # check if powered up
            power_status = self.dev.ctrl_transfer(CC253x.USB_DIR_IN, CC253x.USB_POWER_STATUS, data_or_wLength = 1)
            if power_status[0] == 4:
                break
            time.sleep(0.1)

        # Set the channel in the hardware now the radio is powered up
        self._do_set_channel()

        # Start capture
        self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_START)
        self.__stream_open = True

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        """
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        """
        if self.__stream_open == True:
            # TODO Here, and in other places, add error handling for ctrl_transfer failure
            self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_STOP)
            self.__stream_open = False

    def _do_set_channel(self):
        # Internal function to unconditionally set the channel in the hardware
        # For some CC253x dongles, this can only be done when the radio is powered up.
        self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_CHAN, wIndex = 0, data_or_wLength = [self._channel])
        self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_CHAN, wIndex = 1, data_or_wLength = [0x00])

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel, page=0):
        """
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        self.capabilities.require(KBCapabilities.SETCHAN)

        if channel >= 11 or channel <= 26:
            self._channel = channel
            if self.__stream_open == True:
                self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_STOP)
                self._do_set_channel()
                self.dev.ctrl_transfer(CC253x.USB_DIR_OUT, CC253x.USB_XFER_START)
        else:
            raise Exception('Invalid channel')
        if page:
            raise Exception('SubGHz not supported')

    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        """
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, without FCS.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @type count: Integer
        @param count: Transmits a specified number of frames, def=1
        @type delay: Float
        @param delay: Delay between each frame, def=1
        @rtype: None
        """
        raise Exception('Not yet implemented')

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout=100):
        """
        Returns a dictionary containing packet data, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        """
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing

        ret = None
        framedata = bytearray()
        explen = 0 # expected remaining packet length
        while True:
            pdata = None
            try:
                pdata = self.dev.read(self._data_ep, self._maxPacketSize, timeout=timeout)
            except usb.core.USBError as e:
                if e.errno is not 110 and e.errno is not 60: #Operation timed out
                    print("Error args: {}".format(e.args))
                    raise e
                    #TODO error handling enhancements for USB 1.0
                else:
                    return None

            # Accumulate in 'framedata' until we have an entire frame
            framedata.extend(pdata)

            if len(pdata) < 64:

                if len(pdata) < 2:
                    return None

                framelen = framedata[1]
                if len(framedata) - 3 != framelen:
                    return None

                if framedata[0] != 0:
                    return None

                payloadlen = framedata[7] # Includes TI format FCS
                payload = framedata[8:]

                if len(payload) != payloadlen:
                    # TODO: Log "ERROR: Bad payload length"
                    return None

                # See TI Smart RF User Guide for usage of 'CC24XX' format FCS fields
                # in last two bytes of framedata. Note that we remove these before return of the frame.

                # RSSI is signed value, offset by 73 (see CC2530 data sheet for offset)
                rssi = framedata[-2] - 73
                # Dirty hack to compensate for possible RSSI overflow
                if rssi > 255:
                    rssi = 255 # assumed to be max, could also report error/0

                fcsx = framedata[-1]
                # validcrc is the bit 7 in fcsx
                validcrc  = (fcsx & 0x80) == 0x80
                # correlation value is bits 0-6 in fcsx
                correlation = fcsx & 0x7f

                ret = {1:validcrc, 2:rssi,
                        'validcrc':validcrc, 'rssi':rssi, 'lqi':correlation,
                        'dbm':rssi,'datetime':datetime.utcnow()}

                # Convert the framedata to a string for the return value, and replace the TI FCS with a real FCS
                # if the radio told us that the FCS had passed validation.
                if validcrc:
                    ret[0] = bytearray_to_bytes(payload[:-2]) + makeFCS(payload[:-2])
                else:
                    ret[0] = bytearray_to_bytes(payload)
                ret['bytes'] = ret[0]
                return ret


    def jammer_on(self, channel=None, page=0, method=None):
        """
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        raise Exception('Not yet implemented')

    def set_sync(self, sync=0xA7):
        """
        Set the register controlling the 802.15.4 PHY sync byte.
        """
        raise Exception('Not yet implemented')

    def jammer_off(self):
        """
        Not yet implemented.
        @return: None
        @rtype: None
        """
        raise Exception('Not yet implemented')
