'''
Support is currently only tested with Zigduino r1.
Zigduino support is contributed by neighbor bx.
If you can test with or can provide us a Zigduino r2 for testing, that would be great.

Items still TODO:
    - sniffer_off() needs to instruct the firmware to stop sending packets
    - calibrate the RSSI reading on the r2 hardware and adjust for it
    - add jamming support
'''

import os # type: ignore
import time # type: ignore
import struct # type: ignore
import time # type: ignore
from datetime import datetime, date, timedelta # type: ignore
from .kbutils import KBCapabilities, makeFCS # type: ignore
from .GoodFETatmel128 import GoodFETatmel128rfa1 # type: ignore

ATMEL_REG_SYNC = 0x0B

class ZIGDUINO:
    def __init__(self, dev):
        '''
        Instantiates the KillerBee class for Zigduino running GoodFET firmware.
        @type dev:   String
        @param dev:  Serial device identifier (ex /dev/ttyUSB0)
        @return: None
        @rtype: None
        '''
        self._channel = None
        self.handle = None
        self.dev = dev
        self.handle = GoodFETatmel128rfa1()
        self.handle.serInit(port=self.dev)
        self.handle.setup()

        self.__stream_open = False
        self.capabilities = KBCapabilities()
        self.__set_capabilities()

    def close(self):
	      self.handle.serClose()
	      self.handle = None

    def check_capability(self, capab):
        return self.capabilities.check(capab)

    def get_capabilities(self):
        return self.capabilities.getlist()

    def __set_capabilities(self):
        '''
        Sets the capability information appropriate for GoodFETAVR client and firmware.
        @rtype: None
        @return: None
        '''
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)
        #self.capabilities.setcapab(KBCapabilities.PHYJAM_REFLEX, True)
        self.capabilities.setcapab(KBCapabilities.SET_SYNC, True)

    # KillerBee expects the driver to implement this function
    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "Zigduino", ""]

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        if channel != None or page:
            self.set_channel(channel, page)

        self.__stream_open = True

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        #TODO actually have firmware stop sending us packets!
        self.__stream_open = False

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel, page=0):
        '''
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        if channel >= 11 or channel <= 26:
            self._channel = channel
            self.handle.RF_setchannel(channel)
        else:
            raise Exception('Invalid channel')

    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        '''
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
        '''
        self.capabilities.require(KBCapabilities.INJECT)

        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel != None:
            self.set_channel(channel, page)
        if page:
            raise Exception('SubGHz not supported')
        self.handle.RF_autocrc(1)               #let the radio add the CRC
        for pnum in range(0, count):
            gfready = [ord(x) for x in packet]  #convert packet string to GoodFET expected integer format
            gfready.insert(0, len(gfready)+2)   #add a length that leaves room for CRC
            self.handle.RF_txpacket(gfready)
            time.sleep(delay)

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout=100):
        '''
        Returns a dictionary containing packet data, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing

        packet = None;
        start = datetime.utcnow()
        while (packet is None and (start + timedelta(microseconds=timeout) > datetime.utcnow())):
            packet = self.handle.RF_rxpacket()
        if packet is None:
            return None
        rssi = self.handle.RF_getrssi() #TODO calibrate            
        frame = packet
        if frame[-2:] == makeFCS(frame[:-2]): validcrc = True
        else: validcrc = False
        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result = {0:frame, 1:validcrc, 2:rssi, 'bytes':frame, 'validcrc':validcrc, 'rssi':rssi, 'location':None}
        result['dbm'] = rssi - 45 #TODO tune specifically to the Tmote platform (does ext antenna need to different?)
        result['datetime'] = datetime.utcnow()
        return result

    def jammer_on(self, channel=None, page=0):
        '''
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        raise Exception('Not yet implemented')

    def set_sync(self, sync=0xA7):
        '''Set the register controlling the 802.15.4 PHY sync byte.'''
        self.capabilities.require(KBCapabilities.SET_SYNC)
        if (sync >> 8) > 0:
            raise Exception("Sync word (%x) must be 1 byte." % sync)
        if (sync & 0x0F) == 0:
            raise Exception("Least-significant nybble in sync (%x) cannot be 0." % sync)
        return self.handle.poke(ATMEL_REG_SYNC, sync)

    def jammer_off(self, channel=None, page=0):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        #TODO implement
        raise Exception('Not yet implemented')

