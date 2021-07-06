'''
Support for the TelosB / Tmote Sky platforms, and close clones.

Utilizes the GoodFET firmware with CCSPI application, and the GoodFET client code.
'''

import os # type: ignore
import time # type: ignore
import struct # type: ignore
import time # type: ignore
from datetime import datetime, timedelta # type: ignore
from .kbutils import KBCapabilities, makeFCS # type: ignore
from .GoodFETCCSPI import GoodFETCCSPI # type: ignore

CC2420_REG_SYNC = 0x14

class TELOSB:
    def __init__(self, dev):
        '''
        Instantiates the KillerBee class for our TelosB/TmoteSky running GoodFET firmware.
        @type dev:   String
        @param dev:  Serial device identifier (ex /dev/ttyUSB0)
        @return: None
        @rtype: None
        '''
        self._channel = None
        self._page = 0
        self.handle = None
        self.dev = dev

        os.environ["board"] = "telosb" #set enviroment variable for GoodFET code to use
        self.handle = GoodFETCCSPI()
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
        Sets the capability information appropriate for GoodFETCCSPI client and firmware.
        @rtype: None
        @return: None
        '''
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)
        self.capabilities.setcapab(KBCapabilities.PHYJAM_REFLEX, True)
        self.capabilities.setcapab(KBCapabilities.SET_SYNC, True)
        return

    # KillerBee expects the driver to implement this function
    #def get_dev_info(self, dev, bus):
    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "TelosB/Tmote", ""]

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        self.handle.RF_promiscuity(1);
        self.handle.RF_autocrc(0);

        if channel != None:
            self.set_channel(channel, page)
        
        self.handle.CC_RFST_RX();

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
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        if channel >= 11 or channel <= 26:
            self._channel = channel
            self.handle.RF_setchan(channel)
        else:
            raise Exception('Invalid channel')
        if page:
            raise Exception('SubGHz not supported')

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

        self.handle.RF_autocrc(1)               #let radio add the CRC
        for pnum in range(0, count):
            gfready = list(bytearray(packet))  #convert packet string to GoodFET expected integer format
            gfready.insert(0, len(gfready)+2)   #add a length that leaves room for CRC
            self.handle.RF_txpacket(gfready)
            # Sleep was for 1 second but testing by Gianfranco Costamagna suggested lowering to 1/100th of a second
            time.sleep(0.01)                    #TODO get rid of completely, and just check CC2420 status
            # https://github.com/alvarop/msp430-cc2500/blob/master/lib/cc2500/cc2500.c

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
            rssi = self.handle.RF_getrssi() #TODO calibrate

        if packet is None:
            return None

        frame = packet[1:]
        if frame[-2:] == makeFCS(frame[:-2]): validcrc = True
        else: validcrc = False
        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result = {0:frame, 1:validcrc, 2:rssi, 'bytes':frame, 'validcrc':validcrc, 'rssi':rssi, 'location':None}
        result['dbm'] = rssi - 45 #TODO tune specifically to the Tmote platform (does ext antenna need to different?)
        result['datetime'] = datetime.utcnow()
        return result
 
    def ping(self, da, panid, sa, channel=None, page=0):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        raise Exception('Not yet implemented')

    def jammer_on(self, channel=None, page=0, method=None):
        '''
        Implements reflexive jamming.
        Targeted frames must be >12 bytes for reliable jamming in current firmware.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not support on this device
        @rtype: None
        '''

        if channel is not None:
            self.set_channel(channel, page)

        if method is None or method not in ["reflexive"]:
            raise ValueError("Jamming method is unsupported by this driver.")

        self.capabilities.require(KBCapabilities.PHYJAM_REFLEX)

        self.handle.RF_promiscuity(1)
        self.handle.RF_autocrc(0)

        self.handle.CC_RFST_RX()
        self.handle.RF_reflexjam()

    def set_sync(self, sync=0xA70F):
        '''Set the register controlling the 802.15.4 PHY sync byte.'''
        self.capabilities.require(KBCapabilities.SET_SYNC)
        if (sync >> 16) > 0:
            raise Exception("Sync word (%x) must be 2-bytes or less." % sync)
        return self.handle.poke(CC2420_REG_SYNC, sync)

    def jammer_off(self):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        #TODO implement
        raise Exception('Not yet implemented')

